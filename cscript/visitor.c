#include "visitor.h"
#include "memory.h"

static int previsit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_statements(cscript_context* ctxt, cscript_visitor* v, cscript_comma_separated_statements* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_statements(cscript_context* ctxt, cscript_visitor* v, cscript_comma_separated_statements* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_for(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_for* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_for(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_for* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_if(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_if* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_if(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_if* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static void visit_number(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_number* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_term(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_term* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_term(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_term* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_relop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_relop* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_relop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_relop* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_function(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_function* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_function(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_function* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_factor(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_factor* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_factor(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_factor* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static int previsit_expression_list(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression_list* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  return 1;
  }
static void postvisit_expression_list(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression_list* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static void visit_nop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_nop* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }
static void visit_parameter(cscript_context* ctxt, cscript_visitor* v, cscript_parameter* s)
  {
  UNUSED(ctxt);
  UNUSED(v);
  UNUSED(s);
  }

static cscript_visitor_entry make_entry(void* e, enum cscript_visitor_entry_type t)
  {
  cscript_visitor_entry entry;
  entry.entry = e;
  entry.type = t;
  return entry;
  }

static void destroy(cscript_context* ctxt, cscript_visitor* v)
  {
  if (v != NULL)
    {
    cscript_vector_destroy(ctxt, &(v->v));
    cscript_delete(ctxt, v);
    }
  }

cscript_visitor* cscript_visitor_new(cscript_context* ctxt, void* impl)
  {
  cscript_visitor* v = cscript_new(ctxt, cscript_visitor);
  v->impl = impl;
  v->previsit_statement = previsit_statement;
  v->postvisit_statement = postvisit_statement;
  v->previsit_statements = previsit_statements;
  v->postvisit_statements = postvisit_statements;
  v->previsit_expression = previsit_expression;
  v->postvisit_expression = postvisit_expression;
  v->previsit_assignment = previsit_assignment;
  v->postvisit_assignment = postvisit_assignment;
  v->visit_number = visit_number;
  v->previsit_term = previsit_term;
  v->postvisit_term = postvisit_term;
  v->previsit_relop = previsit_relop;
  v->postvisit_relop = postvisit_relop;
  v->previsit_lvalueop = previsit_lvalueop;
  v->postvisit_lvalueop = postvisit_lvalueop;
  v->previsit_var = previsit_var;
  v->postvisit_var = postvisit_var;
  v->previsit_fixnum = previsit_fixnum;
  v->postvisit_fixnum = postvisit_fixnum;
  v->previsit_flonum = previsit_flonum;
  v->postvisit_flonum = postvisit_flonum;
  v->previsit_factor = previsit_factor;
  v->postvisit_factor = postvisit_factor;
  v->previsit_if = previsit_if;
  v->postvisit_if = postvisit_if;
  v->previsit_for = previsit_for;
  v->postvisit_for = postvisit_for;
  v->previsit_function = previsit_function;
  v->postvisit_function = postvisit_function;
  v->previsit_expression_list = previsit_expression_list;
  v->postvisit_expression_list = postvisit_expression_list;

  v->visit_parameter = visit_parameter;
  v->visit_nop = visit_nop;


  v->destroy = destroy;
  cscript_vector_init(ctxt, &(v->v), cscript_visitor_entry);
  return v;
  }


static void visit_entry(cscript_context* ctxt, cscript_visitor* vis, cscript_visitor_entry* entry_ptr)
  {
  const cscript_visitor_entry e = *entry_ptr; // make a value object, because we'll push back in the visitor vector so that the pointer make become invalid
  switch (e.type)
    {
    case CSCRIPT_VISITOR_STATEMENT_PRE:
    {
    if (vis->previsit_statement(ctxt, vis, cast(cscript_statement*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_STATEMENT_POST), cscript_visitor_entry);
      switch (cast(cscript_statement*, e.entry)->type)
        {
        case cscript_statement_type_expression:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.expr, CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_comma_separated:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.stmts, CSCRIPT_VISITOR_STATEMENTS_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_fixnum:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.fixnum, CSCRIPT_VISITOR_FIXNUM_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_flonum:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.flonum, CSCRIPT_VISITOR_FLONUM_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_nop:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.nop, CSCRIPT_VISITOR_NOP), cscript_visitor_entry);
          break;
        case cscript_statement_type_assignment:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.assignment, CSCRIPT_VISITOR_ASSIGNMENT_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_if:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.iftest, CSCRIPT_VISITOR_IF_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_for:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.forloop, CSCRIPT_VISITOR_FOR_PRE), cscript_visitor_entry);
          break;
        default:
          cscript_assert(0); // not implemented yet
          break;
        }
      break;
      }
    }
    case CSCRIPT_VISITOR_STATEMENT_POST:
    {
    vis->postvisit_statement(ctxt, vis, cast(cscript_statement*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_EXPRESSION_PRE:
    {
    if (vis->previsit_expression(ctxt, vis, cast(cscript_parsed_expression*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_EXPRESSION_POST), cscript_visitor_entry);
      cscript_vector* operands = &(cast(cscript_parsed_expression*, e.entry)->operands);
      cscript_parsed_relop* expr_it = cscript_vector_begin(operands, cscript_parsed_relop);
      cscript_parsed_relop* expr_it_end = cscript_vector_end(operands, cscript_parsed_relop);
      cscript_parsed_relop* expr_rit = expr_it_end - 1;
      cscript_parsed_relop* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_RELOP_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_EXPRESSION_POST:
    {
    vis->postvisit_expression(ctxt, vis, cast(cscript_parsed_expression*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_STATEMENTS_PRE:
    {
    if (vis->previsit_statements(ctxt, vis, cast(cscript_comma_separated_statements*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_STATEMENTS_POST), cscript_visitor_entry);
      cscript_statement* stmt_it = cscript_vector_begin(&cast(cscript_statement*, e.entry)->statement.stmts.statements, cscript_statement);
      cscript_statement* stmt_it_end = cscript_vector_end(&cast(cscript_statement*, e.entry)->statement.stmts.statements, cscript_statement);
      cscript_statement* stmt_rit = stmt_it_end - 1;
      cscript_statement* stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_STATEMENTS_POST:
    {
    vis->postvisit_statements(ctxt, vis, cast(cscript_comma_separated_statements*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_NUMBER:
    {
    vis->visit_number(ctxt, vis, cast(cscript_parsed_number*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_TERM_PRE:
    {
    if (vis->previsit_term(ctxt, vis, cast(cscript_parsed_term*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_TERM_POST), cscript_visitor_entry);
      cscript_vector* operands = &(cast(cscript_parsed_term*, e.entry)->operands);
      cscript_parsed_factor* expr_it = cscript_vector_begin(operands, cscript_parsed_factor);
      cscript_parsed_factor* expr_it_end = cscript_vector_end(operands, cscript_parsed_factor);
      cscript_parsed_factor* expr_rit = expr_it_end - 1;
      cscript_parsed_factor* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_FACTOR_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_TERM_POST:
    {
    vis->postvisit_term(ctxt, vis, cast(cscript_parsed_term*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_RELOP_PRE:
    {
    if (vis->previsit_relop(ctxt, vis, cast(cscript_parsed_relop*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_RELOP_POST), cscript_visitor_entry);
      cscript_vector* operands = &(cast(cscript_parsed_term*, e.entry)->operands);
      cscript_parsed_term* expr_it = cscript_vector_begin(operands, cscript_parsed_term);
      cscript_parsed_term* expr_it_end = cscript_vector_end(operands, cscript_parsed_term);
      cscript_parsed_term* expr_rit = expr_it_end - 1;
      cscript_parsed_term* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_TERM_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_RELOP_POST:
    {
    vis->postvisit_relop(ctxt, vis, cast(cscript_parsed_relop*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_ASSIGNMENT_PRE:
    {
    if (vis->previsit_assignment(ctxt, vis, cast(cscript_parsed_assignment*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_ASSIGNMENT_POST), cscript_visitor_entry);
      cscript_vector* dims = &(cast(cscript_parsed_assignment*, e.entry)->dims);
      cscript_parsed_expression* expr_it = cscript_vector_begin(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, &cast(cscript_parsed_assignment*, e.entry)->expr), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
      }
    break;
    }
    case CSCRIPT_VISITOR_ASSIGNMENT_POST:
    {
    vis->postvisit_assignment(ctxt, vis, cast(cscript_parsed_assignment*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_VAR_PRE:
    {
    if (vis->previsit_var(ctxt, vis, cast(cscript_parsed_variable*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_VAR_POST), cscript_visitor_entry);
      cscript_vector* dims = &(cast(cscript_parsed_variable*, e.entry)->dims);
      cscript_parsed_expression* expr_it = cscript_vector_begin(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_VAR_POST:
    {
    vis->postvisit_var(ctxt, vis, cast(cscript_parsed_variable*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_NOP:
    {
    vis->visit_nop(ctxt, vis, cast(cscript_parsed_nop*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_FIXNUM_PRE:
    {
    if (vis->previsit_fixnum(ctxt, vis, cast(cscript_parsed_fixnum*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_FIXNUM_POST), cscript_visitor_entry);
      cscript_vector* dims = &(cast(cscript_parsed_fixnum*, e.entry)->dims);
      cscript_parsed_expression* expr_it = cscript_vector_begin(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, &cast(cscript_parsed_fixnum*, e.entry)->expr), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
      }
    break;
    }
    case CSCRIPT_VISITOR_FIXNUM_POST:
    {
    vis->postvisit_fixnum(ctxt, vis, cast(cscript_parsed_fixnum*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_FLONUM_PRE:
    {
    if (vis->previsit_flonum(ctxt, vis, cast(cscript_parsed_flonum*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_FIXNUM_POST), cscript_visitor_entry);
      cscript_vector* dims = &(cast(cscript_parsed_flonum*, e.entry)->dims);
      cscript_parsed_expression* expr_it = cscript_vector_begin(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(dims, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, &cast(cscript_parsed_flonum*, e.entry)->expr), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
      }
    break;
    }
    case CSCRIPT_VISITOR_FLONUM_POST:
    {
    vis->postvisit_flonum(ctxt, vis, cast(cscript_parsed_flonum*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_FUNCTION_PRE:
    {
    if (vis->previsit_function(ctxt, vis, cast(cscript_parsed_function*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_FUNCTION_POST), cscript_visitor_entry);
      cscript_vector* args = &(cast(cscript_parsed_function*, e.entry)->args);
      cscript_parsed_expression* expr_it = cscript_vector_begin(args, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(args, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_FUNCTION_POST:
    {
    vis->postvisit_function(ctxt, vis, cast(cscript_parsed_function*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_EXPRESSIONLIST_PRE:
    {
    if (vis->previsit_expression_list(ctxt, vis, cast(cscript_parsed_expression_list*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_EXPRESSIONLIST_POST), cscript_visitor_entry);
      cscript_vector* exprs = &(cast(cscript_parsed_expression_list*, e.entry)->expressions);
      cscript_parsed_expression* expr_it = cscript_vector_begin(exprs, cscript_parsed_expression);
      cscript_parsed_expression* expr_it_end = cscript_vector_end(exprs, cscript_parsed_expression);
      cscript_parsed_expression* expr_rit = expr_it_end - 1;
      cscript_parsed_expression* expr_rit_end = expr_it - 1;
      for (; expr_rit != expr_rit_end; --expr_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, expr_rit), CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_EXPRESSIONLIST_POST:
    {
    vis->postvisit_expression_list(ctxt, vis, cast(cscript_parsed_expression_list*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_FACTOR_PRE:
    {
    if (vis->previsit_factor(ctxt, vis, cast(cscript_parsed_factor*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_FACTOR_POST), cscript_visitor_entry);
      switch (cast(cscript_parsed_factor*, e.entry)->type)
        {
        case cscript_factor_type_number:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.number, CSCRIPT_VISITOR_NUMBER), cscript_visitor_entry);
          break;
        case cscript_factor_type_expression:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.expr, CSCRIPT_VISITOR_EXPRESSION_PRE), cscript_visitor_entry);
          break;
        case cscript_factor_type_variable:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.var, CSCRIPT_VISITOR_VAR_PRE), cscript_visitor_entry);
          break;
        case cscript_factor_type_lvalue_operator:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.lvop, CSCRIPT_VISITOR_LVALUEOP_PRE), cscript_visitor_entry);
          break;
        case cscript_factor_type_function:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.fun, CSCRIPT_VISITOR_FUNCTION_PRE), cscript_visitor_entry);
          break;
        case cscript_factor_type_expression_list:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.exprlist, CSCRIPT_VISITOR_EXPRESSIONLIST_PRE), cscript_visitor_entry);
          break;
        default:
          cscript_assert(0); // not implemented
          break;
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_FACTOR_POST:
    {
    vis->postvisit_factor(ctxt, vis, cast(cscript_parsed_factor*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_PARAMETER:
    {
    vis->visit_parameter(ctxt, vis, cast(cscript_parameter*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_LVALUEOP_PRE:
    {
    if (vis->previsit_lvalueop(ctxt, vis, cast(cscript_parsed_lvalue_operator*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_LVALUEOP_POST), cscript_visitor_entry);
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_lvalue_operator*, e.entry)->lvalue, CSCRIPT_VISITOR_VAR_PRE), cscript_visitor_entry);
      }
    break;
    }
    case CSCRIPT_VISITOR_LVALUEOP_POST:
    {
    vis->postvisit_lvalueop(ctxt, vis, cast(cscript_parsed_lvalue_operator*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_FOR_PRE:
    {
    if (vis->previsit_for(ctxt, vis, cast(cscript_parsed_for*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_FOR_POST), cscript_visitor_entry);
      cscript_statement* stmt_it = cscript_vector_begin(&cast(cscript_parsed_for*, e.entry)->statements, cscript_statement);
      cscript_statement* stmt_it_end = cscript_vector_end(&cast(cscript_parsed_for*, e.entry)->statements, cscript_statement);
      cscript_statement* stmt_rit = stmt_it_end - 1;
      cscript_statement* stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      stmt_it = cscript_vector_begin(&cast(cscript_parsed_for*, e.entry)->init_cond_inc, cscript_statement);
      stmt_it_end = cscript_vector_end(&cast(cscript_parsed_for*, e.entry)->init_cond_inc, cscript_statement);
      stmt_rit = stmt_it_end - 1;
      stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_FOR_POST:
    {
    vis->postvisit_for(ctxt, vis, cast(cscript_parsed_for*, e.entry));
    break;
    }
    case CSCRIPT_VISITOR_IF_PRE:
    {
    if (vis->previsit_if(ctxt, vis, cast(cscript_parsed_if*, e.entry)))
      {
      cscript_vector_push_back(ctxt, &(vis->v), make_entry(e.entry, CSCRIPT_VISITOR_IF_POST), cscript_visitor_entry);
      cscript_statement* stmt_it = cscript_vector_begin(&cast(cscript_parsed_if*, e.entry)->alternative, cscript_statement);
      cscript_statement* stmt_it_end = cscript_vector_end(&cast(cscript_parsed_if*, e.entry)->alternative, cscript_statement);
      cscript_statement* stmt_rit = stmt_it_end - 1;
      cscript_statement* stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      stmt_it = cscript_vector_begin(&cast(cscript_parsed_if*, e.entry)->body, cscript_statement);
      stmt_it_end = cscript_vector_end(&cast(cscript_parsed_if*, e.entry)->body, cscript_statement);
      stmt_rit = stmt_it_end - 1;
      stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      stmt_it = cscript_vector_begin(&cast(cscript_parsed_if*, e.entry)->condition, cscript_statement);
      stmt_it_end = cscript_vector_end(&cast(cscript_parsed_if*, e.entry)->condition, cscript_statement);
      stmt_rit = stmt_it_end - 1;
      stmt_rit_end = stmt_it - 1;
      for (; stmt_rit != stmt_rit_end; --stmt_rit) // IMPORTANT: brackets necessary, as cscript_vector_push_back is a C macro
        {
        cscript_vector_push_back(ctxt, &(vis->v), make_entry(cast(void*, stmt_rit), CSCRIPT_VISITOR_STATEMENT_PRE), cscript_visitor_entry);
        }
      }
    break;
    }
    case CSCRIPT_VISITOR_IF_POST:
    {
    vis->postvisit_if(ctxt, vis, cast(cscript_parsed_if*, e.entry));
    break;
    }
    default:
      cscript_assert(0); // not implemented yet
    }
  }

static void visit(cscript_context* ctxt, cscript_visitor* vis)
  {
  while (vis->v.vector_size > 0)
    {
    cscript_visitor_entry* e = cscript_vector_back(&(vis->v), cscript_visitor_entry);
    cscript_vector_pop_back(&(vis->v));
    visit_entry(ctxt, vis, e);
    }
  }

void cscript_visit_statement(cscript_context* ctxt, cscript_visitor* vis, cscript_statement* stmt)
  {
  cscript_visitor_entry e = make_entry(stmt, CSCRIPT_VISITOR_EXPRESSION_PRE);
  cscript_vector_push_back(ctxt, &(vis->v), e, cscript_visitor_entry);
  visit(ctxt, vis);
  }

void cscript_visit_program(cscript_context* ctxt, cscript_visitor* vis, cscript_program* p)
  {
  cscript_statement* stmt_it = cscript_vector_begin(&p->statements, cscript_statement);
  cscript_statement* stmt_it_end = cscript_vector_end(&p->statements, cscript_statement);
  cscript_statement* stmt_rit = stmt_it_end - 1;
  cscript_statement* stmt_rit_end = stmt_it - 1;
  for (; stmt_rit != stmt_rit_end; --stmt_rit)
    {
    cscript_visitor_entry e = make_entry(stmt_rit, CSCRIPT_VISITOR_STATEMENT_PRE);
    cscript_vector_push_back(ctxt, &(vis->v), e, cscript_visitor_entry);
    }

  cscript_parameter* par_it = cscript_vector_begin(&p->parameters, cscript_parameter);
  cscript_parameter* par_it_end = cscript_vector_end(&p->parameters, cscript_parameter);
  cscript_parameter* par_rit = par_it_end - 1;
  cscript_parameter* par_rit_end = par_it - 1;
  for (; par_rit != par_rit_end; --par_rit)
    {
    cscript_visitor_entry e = make_entry(par_rit, CSCRIPT_VISITOR_PARAMETER);
    cscript_vector_push_back(ctxt, &(vis->v), e, cscript_visitor_entry);
    }

  visit(ctxt, vis);
  }