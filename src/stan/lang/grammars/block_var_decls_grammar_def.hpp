#ifndef STAN_LANG_GRAMMARS_BLOCK_VAR_DECLS_GRAMMAR_DEF_HPP
#define STAN_LANG_GRAMMARS_BLOCK_VAR_DECLS_GRAMMAR_DEF_HPP

#include <stan/lang/ast.hpp>
#include <stan/lang/grammars/block_var_decls_grammar.hpp>
#include <stan/lang/grammars/common_adaptors_def.hpp>
#include <stan/lang/grammars/semantic_actions.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/phoenix.hpp>
#include <boost/version.hpp>
#include <set>
#include <string>
#include <vector>

BOOST_FUSION_ADAPT_STRUCT(stan::lang::double_block_var_decl,
                          (stan::lang::double_block_type, type_)
                          (std::string, name_)
                          (stan::lang::expression, def_))

BOOST_FUSION_ADAPT_STRUCT(stan::lang::double_block_type,
                          (stan::lang::range, bounds_))

BOOST_FUSION_ADAPT_STRUCT(stan::lang::int_block_var_decl,
                          (stan::lang::int_block_type, type_)
                          (std::string, name_)
                          (stan::lang::expression, def_))

BOOST_FUSION_ADAPT_STRUCT(stan::lang::int_block_type,
                          (stan::lang::range, bounds_))

namespace stan {

  namespace lang {

    template <typename Iterator>
    block_var_decls_grammar<Iterator>::block_var_decls_grammar(variable_map& var_map,
                                               std::stringstream& error_msgs)
      : block_var_decls_grammar::base_type(var_decls_r),
        var_map_(var_map),
        error_msgs_(error_msgs),
        expression_g(var_map, error_msgs),
        expression07_g(var_map, error_msgs, expression_g) {
      using boost::spirit::qi::_1;
      using boost::spirit::qi::char_;
      using boost::spirit::qi::eps;
      using boost::spirit::qi::lexeme;
      using boost::spirit::qi::lit;
      using boost::spirit::qi::no_skip;
      using boost::spirit::qi::_pass;
      using boost::spirit::qi::_val;
      using boost::spirit::qi::raw;

      using boost::spirit::qi::labels::_a;
      using boost::spirit::qi::labels::_r1;

      using boost::phoenix::begin;
      using boost::phoenix::end;

      var_decls_r.name("variable declarations");
      var_decls_r
        %= eps[set_var_scope_f(_a, transformed_parameter_origin)]
           > *(var_decl_r(_a));

      // _a = error state local,
      // _r1 var scope
      var_decl_r.name("variable declaration");
      var_decl_r
        = (raw[int_decl_r(_r1)
              [add_block_var_f(_val, _1, boost::phoenix::ref(var_map_), _a, _r1,
                         boost::phoenix::ref(error_msgs))]]
              [add_line_number_f(_val, begin(_1), end(_1))]
           | raw[double_decl_r(_r1)
                [add_block_var_f(_val, _1, boost::phoenix::ref(var_map_), _a, _r1,
                           boost::phoenix::ref(error_msgs_))]]
              [add_line_number_f(_val, begin(_1), end(_1))])
        > eps
          [validate_definition_f(_r1, _val, _pass,
                                 boost::phoenix::ref(error_msgs_))]
        > lit(';');

      // _r1 var scope
      int_decl_r.name("integer declaration");
      int_decl_r
        %= int_type_r(_r1)
        > identifier_r
        > opt_def_r(_r1);

      int_type_r.name("integer type");
      int_type_r
        %= (lit("int")
            >> no_skip[!char_("a-zA-Z0-9_")])
        > -range_brackets_int_r(_r1);

      // _r1 var scope
      double_decl_r.name("real declaration");
      double_decl_r
        %= double_type_r(_r1)
        > identifier_r
        > opt_def_r(_r1);

      double_type_r.name("real type");
      double_type_r
        %= (lit("real")
            >> no_skip[!char_("a-zA-Z0-9_")])
        > -range_brackets_double_r(_r1);

      // _r1 var scope
      opt_def_r.name("variable definition (optional)");
      opt_def_r %= -def_r(_r1);

      // _r1 var scope
      def_r.name("variable definition");
      def_r %= lit('=') > expression_g(_r1);

      // _r1 var scope
      range_brackets_int_r.name("integer range expression pair, brackets");
      range_brackets_int_r
        = lit('<') [empty_range_f(_val, boost::phoenix::ref(error_msgs_))]
        >> (
            ((lit("lower")
              >> lit('=')
              >> expression07_g(_r1)
                 [set_int_range_lower_f(_val, _1, _pass,
                                        boost::phoenix::ref(error_msgs_))])
             >> -(lit(',')
                  >> lit("upper")
                  >> lit('=')
                  >> expression07_g(_r1)
                     [set_int_range_upper_f(_val, _1, _pass,
                                            boost::phoenix::ref(error_msgs_))]))
           |
           (lit("upper")
            >> lit('=')
            >> expression07_g(_r1)
               [set_int_range_upper_f(_val, _1, _pass,
                                      boost::phoenix::ref(error_msgs_))])
            )
        >> lit('>');

      // _r1 var scope
      range_brackets_double_r.name("real range expression pair, brackets");
      range_brackets_double_r
        = lit('<')[empty_range_f(_val, boost::phoenix::ref(error_msgs_))]
        > (
           ((lit("lower")
             > lit('=')
             > expression07_g(_r1)
               [set_double_range_lower_f(_val, _1, _pass,
                                         boost::phoenix::ref(error_msgs_))])
             > -(lit(',')
                 > lit("upper")
                 > lit('=')
                 > expression07_g(_r1)
                   [set_double_range_upper_f(_val, _1, _pass,
                                         boost::phoenix::ref(error_msgs_))]))
           |
           (lit("upper")
            > lit('=')
            > expression07_g(_r1)
              [set_double_range_upper_f(_val, _1, _pass,
                                        boost::phoenix::ref(error_msgs_))])
            )
        > lit('>');

      identifier_r.name("identifier");
      identifier_r
        %= identifier_name_r
           [validate_identifier_f(_val, _pass,
                                  boost::phoenix::ref(error_msgs_))];

      identifier_name_r.name("identifier subrule");
      identifier_name_r
        %= lexeme[char_("a-zA-Z")
                  >> *char_("a-zA-Z0-9_.")];
    }
  }


}
#endif