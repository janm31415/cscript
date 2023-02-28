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
static void visit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* s)
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
static void visit_nop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_nop* s)
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
  v->previsit_expression = previsit_expression;
  v->postvisit_expression = postvisit_expression;
  v->visit_number = visit_number;
  v->previsit_term = previsit_term;
  v->postvisit_term = postvisit_term;
  v->previsit_relop = previsit_relop;
  v->postvisit_relop = postvisit_relop;
  v->visit_var = visit_var;
  v->previsit_fixnum = previsit_fixnum;
  v->postvisit_fixnum = postvisit_fixnum;
  v->previsit_flonum = previsit_flonum;
  v->postvisit_flonum = postvisit_flonum;
  v->previsit_factor = previsit_factor;
  v->postvisit_factor = postvisit_factor;

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
        case cscript_statement_type_fixnum:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.fixnum, CSCRIPT_VISITOR_FIXNUM_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_flonum:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.flonum, CSCRIPT_VISITOR_FLONUM_PRE), cscript_visitor_entry);
          break;
        case cscript_statement_type_nop:
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_statement*, e.entry)->statement.nop, CSCRIPT_VISITOR_NOP), cscript_visitor_entry);
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
    case CSCRIPT_VISITOR_VAR:
    {
    vis->visit_var(ctxt, vis, cast(cscript_parsed_variable*, e.entry));
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
          cscript_vector_push_back(ctxt, &(vis->v), make_entry(&cast(cscript_parsed_factor*, e.entry)->factor.var, CSCRIPT_VISITOR_VAR), cscript_visitor_entry);
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
  visit(ctxt, vis);
  }