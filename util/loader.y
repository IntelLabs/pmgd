%{
#include <stdio.h>
#include <string.h>
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
    void set_property(const char *id, const Jarvis::Property &p) {
        if (_is_node)
            _node->set_property(id, p);
        else
            _edge->set_property(id, p);
    }
} current;

static Jarvis::Node *get_node(Jarvis::Graph &db, long long id,
                              Jarvis::StringID *tag,
                              std::function<void(Jarvis::Node &)> node_func);
static Jarvis::Node *get_node(Jarvis::Graph &db, const char *id,
                              Jarvis::StringID *tag,
                              std::function<void(Jarvis::Node &)> node_func);
%}

/* Causes parser to generate more detailed error messages for syntax errors. */
%error-verbose

%parse-param { yy_params params }

/* Note: because this is a union, no object types with constructors may
 * be used, so all object types are referenced with pointers.
 * All pointers point to objects or strings allocated with new and
 * must be deleted at the point of use.
 * The scanner returns token types STRING and QUOTED_STRING;
 * in both cases the scanner allocates the space with new. */
%union {
    long long i;
    char *s;
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
%type <s> property_id

%destructor { delete $$; } STRING
%destructor { delete $$; } QUOTED_STRING
%destructor { delete $$; } property_id
%destructor { delete $$; } tag

%%

s:        node_or_edge
        | s node_or_edge
        ;

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
              { $$ = current = get_node(params.db, $1, $2, params.node_func); }
        | STRING tag
              { $$ = current = get_node(params.db, $1, $2, params.node_func); }
        ;

tag :     /* empty */ { $$ = new Jarvis::StringID(0); }
        | '#' STRING { $$ = new Jarvis::StringID($2); }
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
              { current.set_property($1, $3); }

        | property_id '=' QUOTED_STRING
              { current.set_property($1, $3); }

        | property_id '=' TRUE
              { current.set_property($1, true); }

        | property_id '=' FALSE
              { current.set_property($1, false); }
        ;

property_id: STRING;

%%

using namespace Jarvis;

static const char ID_STR[] = "jarvis.loader.id";
static bool index_created = false;

#undef Exception

void load(Graph &db, const char *filename,
          std::function<void(Node &)> node_func,
          std::function<void(Edge &)> edge_func)
{
    FILE *f = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
    if (f == NULL)
        throw Jarvis::Exception(201, "load failed", errno, filename, __FILE__, __LINE__);

    load(db, f, node_func, edge_func);
}

void load(Graph &db, FILE *f,
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
    Transaction tx(db, Transaction::ReadWrite);
    yy_params params = { db, node_func, edge_func };
    yyparse(params);
    tx.commit();
}


static Node *get_node(Graph &db, long long id, Jarvis::StringID *tag,
                      std::function<void(Node &)> node_func)
{
    if (!index_created) {
        db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::t_integer);
        index_created = true;
    }

    NodeIterator nodes
        = db.get_nodes(0, PropertyPredicate(ID_STR, PropertyPredicate::Eq, id));
    if (nodes) return &*nodes;

    // Node not found; add it
    Node &node = db.add_node(*tag);
    node.set_property(ID_STR, id);
    if (node_func)
        node_func(node);
    return &node;
}

static Node *get_node(Graph &db, const char *id, Jarvis::StringID *tag,
                      std::function<void(Node &)> node_func)
{
    if (!index_created) {
        db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::t_string);
        index_created = true;
    }

    NodeIterator nodes
        = db.get_nodes(0, PropertyPredicate(ID_STR, PropertyPredicate::Eq, id));
    if (nodes) return &*nodes;

    // Node not found; add it
    Node &node = db.add_node(*tag);
    node.set_property(ID_STR, id);
    if (node_func)
        node_func(node);
    return &node;
}

int yyerror(yy_params, const char *err)
{
    throw Jarvis::Exception(202, "load failed", err, __FILE__, __LINE__);
}
