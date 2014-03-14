%{
#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

extern int yylex();
extern int yyerror(Jarvis::Graph &, const char *);

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_create_buffer(FILE *, int size);
extern void yy_switch_to_buffer(YY_BUFFER_STATE);
extern void yy_delete_buffer(YY_BUFFER_STATE);
extern int yyparse(Jarvis::Graph &db);

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

static Jarvis::Node *get_node(Jarvis::Graph &db, long long id);
static Jarvis::Node *get_node(Jarvis::Graph &db, const char *id);
%}

/* Causes parser to generate more detailed error messages for syntax errors. */
%error-verbose

%parse-param { Jarvis::Graph &db }

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
%type <n> node_id
%type <s> property_id

%destructor { delete $$; } STRING
%destructor { delete $$; } QUOTED_STRING
%destructor { delete $$; } property_id

%%

s:        node_or_edge
        | s node_or_edge
        ;

node_or_edge:
          node ';'
        | edge ';'
        ;

node:     node_id properties
        ;

edge:     node_id properties node_id properties
                  { current = db.add_edge(*$1, *$3, 0); }
              edge_properties
        ;

edge_properties:
          /* empty */
        | ':' '{' propertylist '}'
        ;

node_id:  INTEGER { $$ = current = get_node(db, $1); }
        | STRING { $$ = current = get_node(db, $1); }

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

static const char ID[] = "id";

#undef Exception

void load(Graph &db, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        throw Jarvis::Exception(201, "load_failed", __FILE__, __LINE__);

    load(db, f);
}

void load(Graph &db, FILE *f)
{
    Transaction tx(db);
    YY_BUFFER_STATE buffer = yy_create_buffer(f, 500);
    yy_switch_to_buffer(buffer);
    try { yyparse(db); } catch (...) { }
    yy_delete_buffer(buffer);
    tx.commit();
}


static Node *get_node(Graph &db, long long id)
{
#if 0
    // This API not available yet
    NodeIterator nodes = db.get_nodes(0,
                             PropertyPredicate(ID, PropertyPredicate::eq, id));
    if (nodes) return *nodes;
#else
    NodeIterator i = db.get_nodes();

    NodeIterator j = i.filter([id](const Node &n)
                              { return n.get_property(ID).int_value() == id
                                           ? pass : dont_pass; });
    if (j) return &*j;
#endif

    // Node not found; add it
    Node &node = db.add_node(0);
    node.set_property(ID, id);
    return &node;
}

static Node *get_node(Graph &db, const char *id)
{
#if 0
    // This API not available yet
    NodeIterator nodes = db.get_nodes(0,
                             PropertyPredicate(ID, PropertyPredicate::eq, id));
    if (nodes) return *nodes;
#else
    NodeIterator i = db.get_nodes();

    NodeIterator j = i.filter([id](const Node &n)
                          { return n.get_property(ID).string_value() == id
                                       ? pass : dont_pass; });
    if (j) return &*j;
#endif

    // Node not found; add it
    Node &node = db.add_node(0);
    node.set_property(ID, id);
    return &node;
}

int yyerror(Jarvis::Graph &, const char *err)
{
    printf("err %s\n", err);
    throw Jarvis::Exception(201, "load_failed", __FILE__, __LINE__);
}
