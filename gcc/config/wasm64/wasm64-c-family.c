static hash_set<tree> wasm64_jsexport_declared;
static hash_set<tree> wasm64_jsexport_exported;

void
wasm64_jsexport_init(void)
{
  wasm64_jsexport_declared = hash_set<tree>();
  wasm64_jsexport_exported = hash_set<tree>();
}

const char *
wasm64_jsexport_format_type (tree type)
{
  c_family_pretty_printer cpp;

  cpp.buffer->stream = NULL;
  cpp.buffer->flush_p = false;
  cpp.flags |= pp_c_flag_abstract;

  cpp.type_id (type);

  return concat (output_buffer_formatted_text (cpp.buffer), NULL);
}

void
wasm64_jsexport_declare_type (tree type, struct wasm64_jsexport_opts *opts,
			     vec<struct wasm64_jsexport_decl> *decls)
{
  tree origtype = type;
  while (TREE_CODE (type) == POINTER_TYPE) {
    type = TREE_TYPE (type);
    if (TYPE_CANONICAL (type) && TYPE_CANONICAL (type) != type)
      type = TYPE_CANONICAL (type);
  }

  if (wasm64_jsexport_declared.add(type))
    return;

  wasm64_jsexport_declared.add(origtype);

  if (TREE_CODE (type) == RECORD_TYPE) {
    struct wasm64_jsexport_decl ret;
    ret.fragments = vec<const char *>();
    ret.fragments.safe_push (concat (wasm64_jsexport_format_type (type),
				     ";", NULL));
    decls->safe_push (ret);
  }
}

tree
wasm64_canonicalize_type (tree type)
{
  if (TREE_CODE (type) == POINTER_TYPE)
    return build_pointer_type (wasm64_canonicalize_type (TREE_TYPE (type)));

  if (TYPE_CANONICAL (type))
    type = TYPE_CANONICAL (type);

  return type;
}

void
wasm64_jsexport_type (tree type, struct wasm64_jsexport_opts *opts,
		     vec<struct wasm64_jsexport_decl> *decls);

void
wasm64_jsexport_record_type (tree type, struct wasm64_jsexport_opts *opts,
			     vec<struct wasm64_jsexport_decl> *decls)
{
  if (wasm64_jsexport_exported.add (type))
    return;

  const char *jsname = opts->jsname;
  int recurse = opts->recurse;
  struct wasm64_jsexport_decl ret;

  ret.fragments = vec<const char *>();

  const char *typestr = wasm64_jsexport_format_type (build_pointer_type (type));

  if (!jsname)
    if (type && TYPE_IDENTIFIER (type))
      jsname = concat(IDENTIFIER_POINTER (TYPE_IDENTIFIER (type)), NULL);
    else
      return;

  ret.jsname = jsname;

  wasm64_jsexport_declare_type (type, opts, decls);
  ret.fragments.safe_push(concat ("wasm64_register_record_type(\"", ret.jsname,
			     "\", ", typestr, ");", NULL));

  tree field;

  for (field = TYPE_FIELDS (type); field && TREE_CODE (field) == FIELD_DECL; field = TREE_CHAIN (field))
    {
      tree fieldtype = TREE_TYPE(field);

      if (TYPE_CANONICAL (fieldtype) != fieldtype)
	fieldtype = TYPE_CANONICAL (fieldtype);
      const char *fieldtypestr = wasm64_jsexport_format_type (build_pointer_type (fieldtype));

      /* ignore bitfields */
      if (strchr (fieldtypestr, ':'))
	continue;

      /* ignore anonymous structs and unions */
      if (strstr (fieldtypestr, "<anonymous>"))
	continue;

      char bitpos[16];
      char bytesize[16];
      snprintf(bitpos, 16, "%ld", (long)int_bit_position (field));
      snprintf(bytesize, 16, "%ld", (long)int_size_in_bytes (TREE_TYPE (field)));

      wasm64_jsexport_declare_type (fieldtype, opts, decls);
      wasm64_jsexport_type (fieldtype, opts, decls);

      ret.fragments.safe_push(concat ("wasm64_register_field(\"", ret.jsname,
				      "\", ",
				      IDENTIFIER_POINTER (DECL_NAME (field)),
				      ", ", typestr,
				      ", ", fieldtypestr,
				      ", ", bitpos,
				      ", ", bytesize,
				      ");",
				 NULL));
    }

  decls->safe_push(ret);
}

void
wasm64_jsexport_type (tree type, struct wasm64_jsexport_opts *opts,
		     vec<struct wasm64_jsexport_decl> *decls)
{
  if (TREE_CODE (type) == POINTER_TYPE) {
    wasm64_jsexport_type (TREE_TYPE (type), opts, decls);
    return;
  }
  if (TYPE_CANONICAL (type) != type) {
    wasm64_jsexport_type (TYPE_CANONICAL (type), opts, decls);
    return;
  }
  if (TREE_CODE (type) == RECORD_TYPE) {
    wasm64_jsexport_record_type (type, opts, decls);
    return;
  }
}

void
wasm64_jsexport_function_decl (tree decl, struct wasm64_jsexport_opts *opts,
			      vec<struct wasm64_jsexport_decl> *decls)
{
  struct wasm64_jsexport_decl ret;
  tree type = TREE_TYPE (decl);
  const char *typestr;
  const char *rettypestr;
  const char *classtypestr = "void";
  const char *jsname = opts->jsname;
  int recurse = opts->recurse;

  const char *types = "";
  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      tree arg_types = TYPE_ARG_TYPES (type);
      bool first = true;

      while (arg_types)
	{
	  tree arg_type = TREE_VALUE (arg_types);
	  arg_types = TREE_CHAIN (arg_types);

	  while (TREE_CODE (arg_type) == POINTER_TYPE)
	    {
	      arg_type = TREE_TYPE (arg_type);
	    }

	  if (TREE_CODE (arg_type) != RECORD_TYPE &&
	      TREE_CODE (arg_type) != UNION_TYPE &&
	      TREE_CODE (arg_type) != ENUMERAL_TYPE)
	    {
	      first = false;
	      continue;
	    }

	  if (first)
	    {
	      classtypestr = wasm64_jsexport_format_type (build_pointer_type (arg_type));
	      first = false;
	    }

	  types = concat (types, wasm64_jsexport_format_type (arg_type), "; ", NULL);
	}
    }
  typestr = wasm64_jsexport_format_type (build_pointer_type (type));
  rettypestr = wasm64_jsexport_format_type (TREE_TYPE (type));

  ret.decl = decl;
  if (!jsname && (TREE_CODE (type) == METHOD_TYPE ||
		  TREE_CODE (type) == FUNCTION_TYPE))
    {
      tree basetype = TYPE_METHOD_BASETYPE (type);

      jsname = "";

      while (basetype)
	{
	  switch (TREE_CODE (basetype))
	    {
	    case IDENTIFIER_NODE:
	      if (IDENTIFIER_POINTER (basetype))
		jsname = concat (IDENTIFIER_POINTER (basetype), "::", NULL);
	      basetype = NULL;
	      break;

	    case TYPE_DECL:
	      basetype = DECL_NAME (basetype);
	      break;

	    case RECORD_TYPE:
	      basetype = TYPE_NAME (basetype);
	      break;

	    default:
	      basetype = NULL;
	      break;
	    }
	}

      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }
  else if (!jsname)
    {
      jsname = "";
      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }
  ret.jsname = jsname;
  ret.symbol = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
  ret.fragments = vec<const char *>();
  ret.fragments.safe_push(concat (types,
				  "wasm64_register_function(\"", ret.jsname,
				  "\", ", "(", typestr, ")", NULL));
  ret.fragments.safe_push(NULL);
  ret.fragments.safe_push(concat(", ", classtypestr,
				 ", ", rettypestr,
				 ", ", typestr,
				 ");",
				 NULL));

  decls->safe_push(ret);
}

void
wasm64_jsexport_type_decl (tree node, struct wasm64_jsexport_opts *opts,
			  vec<struct wasm64_jsexport_decl> *decls)
{
  wasm64_jsexport_type (TREE_TYPE (node), opts, decls);
}

void
wasm64_jsexport_var_decl (tree decl, struct wasm64_jsexport_opts *opts,
			 vec<struct wasm64_jsexport_decl> *decls)
{
  struct wasm64_jsexport_decl ret;
  tree type = TREE_TYPE (decl);
  tree attrs = TYPE_ATTRIBUTES (type);
  const char *type_jsname = NULL;

  if (attrs != NULL_TREE)
    {
      tree attr = lookup_attribute ("jsexport", attrs);

      if (attr != NULL_TREE)
	{
	  tree arg1 = TREE_VALUE (attr);

	  if (arg1)
	    type_jsname = TREE_STRING_POINTER (arg1);
	}
    }

  if (!type_jsname)
    {
      tree basetype = type;

      while (basetype)
	{
	  switch (TREE_CODE (basetype))
	    {
	    case IDENTIFIER_NODE:
	      if (IDENTIFIER_POINTER (basetype))
		type_jsname = concat (IDENTIFIER_POINTER (basetype), NULL);
	      basetype = NULL;
	      break;

	    case TYPE_DECL:
	      basetype = DECL_NAME (basetype);
	      break;

	    case RECORD_TYPE:
	      basetype = TYPE_NAME (basetype);
	      break;

	    default:
	      basetype = NULL;
	      break;
	    }
	}
    }

  type = wasm64_canonicalize_type (type);
  const char *typestr = wasm64_jsexport_format_type (build_pointer_type (type));

  if (!type_jsname)
    type_jsname = "";

  const char *jsname = opts->jsname;
  if (!jsname)
    {
      jsname = "";
      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }

  ret.jsname = jsname;

  ret.fragments = vec<const char *>();
  ret.fragments.safe_push (concat ("wasm64_register_variable(\"", ret.jsname,
				   "\", ", typestr,
				   ", \"", type_jsname, "\", ", NULL));
  ret.fragments.safe_push (NULL);
  ret.fragments.safe_push (concat (");", NULL));
  ret.symbol = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));

  decls->safe_push(ret);
}

void
wasm64_jsexport_decl (tree node, struct wasm64_jsexport_opts *opts,
		     vec<struct wasm64_jsexport_decl> *decls)
{
  switch (TREE_CODE (node))
    {
    case FUNCTION_DECL:
      wasm64_jsexport_function_decl (node, opts, decls);
      return;
    case TYPE_DECL:
      wasm64_jsexport_type_decl (node, opts, decls);
      return;
    case VAR_DECL:
      wasm64_jsexport_var_decl (node, opts, decls);
      return;
    default:
      printf("unknown code:\n");
      debug_tree (node);
      gcc_unreachable();
    }
}

void
wasm64_jsexport (tree node, struct wasm64_jsexport_opts *opts,
		vec<struct wasm64_jsexport_decl> *decls)
{
#if 0
  const char *mangled = NULL;
  const char *force_mangled = NULL;

  if (DECL_P (node) && !DECL_ASSEMBLER_NAME_SET_P (node))
    mangle_decl (node);

  if (DECL_P (node))
    {
      mangled = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (node));

      force_mangled = IDENTIFIER_POINTER (get_mangled_id (node, true));
    }
  fprintf(stderr, "name %s mangled %s force-mangled %s\n", DECL_P (node) ? IDENTIFIER_POINTER (DECL_NAME (node)) : NULL,
	  mangled, force_mangled);
#endif

  if (DECL_P (node)) {
    wasm64_jsexport_decl (node, opts, decls);
    return;
  } else if (TYPE_P (node)) {
    wasm64_jsexport_type (node, opts, decls);
    return;
  } else
    {
      error("unknown tree type");
      debug_tree (node);
      gcc_unreachable ();
    }
}
