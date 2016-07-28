%{
#include <stdio.h>
#include <string.h>
#include <map>
#include "jarvis.h"
#include "../util/util.h"
#include "loader.h"

extern int yylex();
extern int yyerror(yy_params, const char *);

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_create_buffer(FILE *, int size);
extern void yy_switch_to_buffer(YY_BUFFER_STATE);
extern void yy_delete_buffer(YY_BUFFER_STATE);
extern int yyparse(yy_params);

static class Current {
    bool _is_node;
    union {
        Jarvis::Node *_node;
        Jarvis::Edge *_edge;
    };

public:
    Current() { }
    Jarvis::Node *operator=(Jarvis::Node *n) { _is_node = true; _node = n; return n; }
    void operator=(Jarvis::Edge &e) { _is_node = false; _edge = &e; }
    void set_property(const Jarvis::StringID *id, const Jarvis::Property &p) {
        if (_is_node)
            _node->set_property(*id, p);
        else
            _edge->set_property(*id, p);
    }
} current;

template <typename T>
static Jarvis::Node *get_node(yy_params &params, const T &id, Jarvis::StringID *tag);
%}

/* Causes parser to generate more detailed error messages for syntax errors. */
%error-verbose

%parse-param { yy_params params }

/* Note: because this is a union, no object types with constructors
 * are used; instead object types are referenced with pointers.
 * The pointers point to objects allocated with new, which must be
 * deleted at the point of use.
 * The scanner returns token types STRING and QUOTED_STRING, both of
 * type std::string, also allocated with new and deleted when used.
 * The Node type is an exception: it is a pointer to an actual Jarvis
 * Node, so it doesn't need to be deleted.
 */
%union {
    long long i;
    std::string *s;
    Jarvis::Node *n;
    Jarvis::StringID *id;
}

%{
extern int yyerror(Jarvis::Graph &, const char *);
%}

%token ERROR

%token TRUE
%token FALSE

/* Tokens with values */
%token <i> INTEGER
%token <s> STRING
%token <s> QUOTED_STRING

/* non-terminals with values */
%type <n> node
%type <id> tag
%type <id> property_id

%destructor { delete $$; } STRING
%destructor { delete $$; } QUOTED_STRING
%destructor { delete $$; } property_id
%destructor { delete $$; } tag

%%

s:        tx
        | s tx
        ;

tx:
          { params.tx = new Jarvis::Transaction(params.db,
                                             Jarvis::Transaction::ReadWrite); }
          node_or_edge
          { params.tx->commit(); delete params.tx; }

node_or_edge:
          node_def ';'
        | edge_def ';'
        ;

node_def:     node properties
        ;

edge_def: node properties node properties
              ':' tag
              {
                  Jarvis::Edge &edge = params.db.add_edge(*$1, *$3, *$6);
                  if (params.edge_func)
                      params.edge_func(edge);
                  current = edge;
                  delete $6;
              }
              edge_properties
        | node properties node properties
              {
                  Jarvis::Edge &edge = params.db.add_edge(*$1, *$3, 0);
                  if (params.edge_func)
                      params.edge_func(edge);
                  current = edge;
              }
        ;

edge_properties:
          /* empty */
        | '{' propertylist '}'
        ;

node:     INTEGER tag
              {
                  $$ = current = get_node(params, $1, $2);
                  delete $2;
              }

        | STRING tag
              {
                  $$ = current = get_node(params, *$1, $2);
                  delete $1;
                  delete $2;
              }
        ;

tag :     /* empty */ { $$ = new Jarvis::StringID(0); }
        | '#' STRING
              {
                  $$ = new Jarvis::StringID($2->c_str());
                  delete $2;
              }
        ;

properties:
          /* empty */
        | '{' propertylist '}'
        ;

propertylist:
          property
        | property opt_comma propertylist
        ;

opt_comma:
          /* empty */
        | ','
        ;

property:
          property_id '=' INTEGER
              {
                  current.set_property($1, $3);
                  delete $1;
              }

        | property_id '=' QUOTED_STRING
              {
                  const std::string &s = *$3;
                  std::string value;
                  std::string::size_type pos;
                  std::string::size_type prev = 0;
                  while ((pos = s.find('\\', prev)) != s.npos) {
                      value.append(s, prev, pos - prev);
                      prev = pos + 1;
                  }
                  value.append(s, prev, s.npos);
                  current.set_property($1, value);
                  delete $1;
                  delete $3;
              }

        | property_id '=' STRING
              {
                  struct tm tm;
                  unsigned long usec;
                  int hr_offset, min_offset;
                  if (!string_to_tm(*$3, &tm, &usec, &hr_offset, &min_offset))
                      throw Jarvis::Exception(LoaderFormatError);
                  Jarvis::Time time(&tm, usec, hr_offset, min_offset);
                  current.set_property($1, time);
                  delete $1;
                  delete $3;
              }

        | property_id '=' TRUE
              {
                  current.set_property($1, true);
                  delete $1;
              }

        | property_id '=' FALSE
              {
                  current.set_property($1, false);
                  delete $1;
              }

        | property_id
              {
                  current.set_property($1, Jarvis::Property());
                  delete $1;
              }
        ;

property_id: STRING
              {
                  $$ = new Jarvis::StringID($1->c_str());
                  delete $1;
              }
        ;

%%

using namespace Jarvis;

static const char ID_STR[] = "jarvis.loader.id";

class Index {
    struct IndexBase {
        virtual ~IndexBase() { }
        virtual Node *find(const long long &id) { return NULL; }
        virtual Node *find(const std::string &id) { return NULL; }
        virtual void add(Node *, const long long &id) { }
        virtual void add(Node *, const std::string &id) { }
    };
    template <typename T> class GraphIndex;
    template <typename T> class MapIndex;

    Graph &_db;
    bool _use_index;
    IndexBase *_index;

public:
    Index(Graph &db, bool use_index)
        : _db(db), _use_index(use_index), _index(NULL)
        { }
    ~Index() { delete _index; }

    template <typename T> Node *find(const T &id);
    template <typename T> void add(Node *node, const T &id)
        { _index->add(node, id); }
};

void load(Graph &db, const char *filename, bool use_index,
          std::function<void(Node &)> node_func,
          std::function<void(Edge &)> edge_func)
{
    FILE *f = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
    if (f == NULL)
        throw Exception(LoaderOpenFailed, errno, filename);

    load(db, f, use_index, node_func, edge_func);
}

void load(Graph &db, FILE *f, bool use_index,
          std::function<void(Node &)> node_func,
          std::function<void(Edge &)> edge_func)
{
    class buffer_t {
        YY_BUFFER_STATE _buffer;
    public:
        buffer_t(FILE *f)
        {
            _buffer = yy_create_buffer(f, 500);
            yy_switch_to_buffer(_buffer);
        }

        ~buffer_t() { yy_delete_buffer(_buffer); }
    } buffer(f);
    Index index(db, use_index);
    yy_params params = { db, index, node_func, edge_func };
    yyparse(params);
}


template <typename T>
static Node *get_node(yy_params &params, const T &id, StringID *tag)
{
    Node *n = params.index.find(id);
    if (n) return n;

    // Node not found; add it
    Node &node = params.db.add_node(*tag);
    params.index.add(&node, id);
    if (params.node_func)
        params.node_func(node);
    return &node;
}

template <typename T>
class Index::GraphIndex : public IndexBase
{
    Graph &_db;

public:
    GraphIndex(Graph &db);

    Node *find(const T &id)
    {
        NodeIterator nodes = _db.get_nodes(0,
            PropertyPredicate(ID_STR, PropertyPredicate::Eq, id));
        if (nodes) return &*nodes;
        return NULL;
    }

    void add(Node *node, const T &id)
    {
        node->set_property(ID_STR, id);
    }
};

template <> Index::GraphIndex<long long>::GraphIndex(Graph &db)
    : _db(db)
{
    _db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::Integer);
}

template <> Index::GraphIndex<std::string>::GraphIndex(Graph &db)
    : _db(db)
{
    _db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::String);
}

template <typename T>
class Index::MapIndex : public IndexBase
{
    typedef std::map<T, Node *> map;
    map node_index;

public:
    Node *find(const T &id)
    {
        typename map::iterator n = node_index.find(id);
        if (n != node_index.end())
            return n->second;
        return NULL;
    }

    void add(Node *node, const T &id)
    {
        node_index.insert(typename map::value_type(id, node));
    }
};

template <typename T> Node *Index::find(const T &id)
{
    if (_index == NULL)
        _index = _use_index
                     ? static_cast<IndexBase *>(new GraphIndex<T>(_db))
                     : static_cast<IndexBase *>(new MapIndex<T>());
    return _index->find(id);
}


int yyerror(yy_params, const char *err)
{
    throw Exception(LoaderParseError, err);
}
