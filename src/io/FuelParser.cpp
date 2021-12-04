// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// First part of user declarations.

#line 37 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:404

#ifndef YY_NULLPTR
#    if defined __cplusplus && 201103L <= __cplusplus
#        define YY_NULLPTR nullptr
#    else
#        define YY_NULLPTR 0
#    endif
#endif

#include "FuelParser.hpp"

// User implementation prologue.

#line 51 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:412
// Unqualified %code blocks.
#line 37 "src/FuelParser.y" // lalr1.cc:413

static int yylex(std::string* yylval, ehb::FuelScanner& scanner)
{
    return scanner.scan(yylval);
}

#line 62 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:413

#ifndef YY_
#    if defined YYENABLE_NLS && YYENABLE_NLS
#        if ENABLE_NLS
#            include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#            define YY_(msgid) dgettext("bison-runtime", msgid)
#        endif
#    endif
#    ifndef YY_
#        define YY_(msgid) msgid
#    endif
#endif

// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void)(E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
#    define YYCDEBUG \
        if (yydebug_) (*yycdebug_)

#    define YY_SYMBOL_PRINT(Title, Symbol)     \
        do                                     \
        {                                      \
            if (yydebug_)                      \
            {                                  \
                *yycdebug_ << Title << ' ';    \
                yy_print_(*yycdebug_, Symbol); \
                *yycdebug_ << std::endl;       \
            }                                  \
        } while (false)

#    define YY_REDUCE_PRINT(Rule)                 \
        do                                        \
        {                                         \
            if (yydebug_) yy_reduce_print_(Rule); \
        } while (false)

#    define YY_STACK_PRINT()                \
        do                                  \
        {                                   \
            if (yydebug_) yystack_print_(); \
        } while (false)

#else // !YYDEBUG

#    define YYCDEBUG \
        if (false) std::cerr
#    define YY_SYMBOL_PRINT(Title, Symbol) YYUSE(Symbol)
#    define YY_REDUCE_PRINT(Rule) static_cast<void>(0)
#    define YY_STACK_PRINT() static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok (yyerrstatus_ = 0)
#define yyclearin (yyla.clear())

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYRECOVERING() (!!yyerrstatus_)

#line 24 "src/FuelParser.y" // lalr1.cc:479
namespace ehb
{
#line 129 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:479

    /// Build a parser object.
    FuelParser ::FuelParser(ehb::FuelScanner& scanner_yyarg, ehb::FuelBlock* node_yyarg) :
#if YYDEBUG
        yydebug_(false),
        yycdebug_(&std::cerr),
#endif
        scanner(scanner_yyarg),
        node(node_yyarg)
    {
    }

    FuelParser ::~FuelParser() {}

    /*---------------.
  | Symbol types.  |
  `---------------*/

    inline FuelParser ::syntax_error::syntax_error(const std::string& m) :
        std::runtime_error(m) {}

    // basic_symbol.
    template<typename Base>
    inline FuelParser ::basic_symbol<Base>::basic_symbol() :
        value() {}

    template<typename Base>
    inline FuelParser ::basic_symbol<Base>::basic_symbol(const basic_symbol& other) :
        Base(other), value() { value = other.value; }

    template<typename Base>
    inline FuelParser ::basic_symbol<Base>::basic_symbol(typename Base::kind_type t, const semantic_type& v) :
        Base(t), value(v) {}

    /// Constructor for valueless symbols.
    template<typename Base>
    inline FuelParser ::basic_symbol<Base>::basic_symbol(typename Base::kind_type t) :
        Base(t), value() {}

    template<typename Base>
    inline FuelParser ::basic_symbol<Base>::~basic_symbol() { clear(); }

    template<typename Base>
    inline void FuelParser ::basic_symbol<Base>::clear() { Base::clear(); }

    template<typename Base>
    inline bool FuelParser ::basic_symbol<Base>::empty() const { return Base::type_get() == empty_symbol; }

    template<typename Base>
    inline void FuelParser ::basic_symbol<Base>::move(basic_symbol& s)
    {
        super_type::move(s);
        value = s.value;
    }

    // by_type.
    inline FuelParser ::by_type::by_type() :
        type(empty_symbol) {}

    inline FuelParser ::by_type::by_type(const by_type& other) :
        type(other.type) {}

    inline FuelParser ::by_type::by_type(token_type t) :
        type(yytranslate_(t)) {}

    inline void FuelParser ::by_type::clear() { type = empty_symbol; }

    inline void FuelParser ::by_type::move(by_type& that)
    {
        type = that.type;
        that.clear();
    }

    inline int FuelParser ::by_type::type_get() const { return type; }

    // by_state.
    inline FuelParser ::by_state::by_state() :
        state(empty_state) {}

    inline FuelParser ::by_state::by_state(const by_state& other) :
        state(other.state) {}

    inline void FuelParser ::by_state::clear() { state = empty_state; }

    inline void FuelParser ::by_state::move(by_state& that)
    {
        state = that.state;
        that.clear();
    }

    inline FuelParser ::by_state::by_state(state_type s) :
        state(s) {}

    inline FuelParser ::symbol_number_type FuelParser ::by_state::type_get() const
    {
        if (state == empty_state)
            return empty_symbol;
        else
            return yystos_[state];
    }

    inline FuelParser ::stack_symbol_type::stack_symbol_type() {}

    inline FuelParser ::stack_symbol_type::stack_symbol_type(state_type s, symbol_type& that) :
        super_type(s)
    {
        value = that.value;
        // that is emptied.
        that.type = empty_symbol;
    }

    inline FuelParser ::stack_symbol_type& FuelParser ::stack_symbol_type::operator=(const stack_symbol_type& that)
    {
        state = that.state;
        value = that.value;
        return *this;
    }

    template<typename Base>
    inline void FuelParser ::yy_destroy_(const char* yymsg, basic_symbol<Base>& yysym) const
    {
        if (yymsg) YY_SYMBOL_PRINT(yymsg, yysym);

        // User destructor.
        YYUSE(yysym.type_get());
    }

#if YYDEBUG
    template<typename Base>
    void FuelParser ::yy_print_(std::ostream& yyo, const basic_symbol<Base>& yysym) const
    {
        std::ostream& yyoutput = yyo;
        YYUSE(yyoutput);
        symbol_number_type yytype = yysym.type_get();
        // Avoid a (spurious) G++ 4.8 warning about "array subscript is
        // below array bounds".
        if (yysym.empty()) std::abort();
        yyo << (yytype < yyntokens_ ? "token" : "nterm") << ' ' << yytname_[yytype] << " (";
        YYUSE(yytype);
        yyo << ')';
    }
#endif

    inline void FuelParser ::yypush_(const char* m, state_type s, symbol_type& sym)
    {
        stack_symbol_type t(s, sym);
        yypush_(m, t);
    }

    inline void FuelParser ::yypush_(const char* m, stack_symbol_type& s)
    {
        if (m) YY_SYMBOL_PRINT(m, s);
        yystack_.push(s);
    }

    inline void FuelParser ::yypop_(unsigned int n) { yystack_.pop(n); }

#if YYDEBUG
    std::ostream& FuelParser ::debug_stream() const
    {
        return *yycdebug_;
    }

    void FuelParser ::set_debug_stream(std::ostream& o) { yycdebug_ = &o; }

    FuelParser ::debug_level_type FuelParser ::debug_level() const { return yydebug_; }

    void FuelParser ::set_debug_level(debug_level_type l) { yydebug_ = l; }
#endif // YYDEBUG

    inline FuelParser ::state_type FuelParser ::yy_lr_goto_state_(state_type yystate, int yysym)
    {
        int yyr = yypgoto_[yysym - yyntokens_] + yystate;
        if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
            return yytable_[yyr];
        else
            return yydefgoto_[yysym - yyntokens_];
    }

    inline bool FuelParser ::yy_pact_value_is_default_(int yyvalue) { return yyvalue == yypact_ninf_; }

    inline bool FuelParser ::yy_table_value_is_error_(int yyvalue) { return yyvalue == yytable_ninf_; }

    int FuelParser ::parse()
    {
        // State.
        int yyn;
        /// Length of the RHS of the rule being reduced.
        int yylen = 0;

        // Error handling.
        int yynerrs_ = 0;
        int yyerrstatus_ = 0;

        /// The lookahead symbol.
        symbol_type yyla;

        /// The return value of parse ().
        int yyresult;

        // FIXME: This shoud be completely indented.  It is not yet to
        // avoid gratuitous conflicts when merging into the master branch.
        try
        {
            YYCDEBUG << "Starting parse" << std::endl;

            /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
            yystack_.clear();
            yypush_(YY_NULLPTR, 0, yyla);

            // A new symbol was pushed on the stack.
        yynewstate:
            YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

            // Accept?
            if (yystack_[0].state == yyfinal_) goto yyacceptlab;

            goto yybackup;

            // Backup.
        yybackup:

            // Try to take a decision without lookahead.
            yyn = yypact_[yystack_[0].state];
            if (yy_pact_value_is_default_(yyn)) goto yydefault;

            // Read a lookahead token.
            if (yyla.empty())
            {
                YYCDEBUG << "Reading a token: ";
                try
                {
                    yyla.type = yytranslate_(yylex(&yyla.value, scanner));
                }
                catch (const syntax_error& yyexc)
                {
                    error(yyexc);
                    goto yyerrlab1;
                }
            }
            YY_SYMBOL_PRINT("Next token is", yyla);

            /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
            yyn += yyla.type_get();
            if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get()) goto yydefault;

            // Reduce or error.
            yyn = yytable_[yyn];
            if (yyn <= 0)
            {
                if (yy_table_value_is_error_(yyn)) goto yyerrlab;
                yyn = -yyn;
                goto yyreduce;
            }

            // Count tokens shifted since error; after three, turn off error status.
            if (yyerrstatus_) --yyerrstatus_;

            // Shift the lookahead token.
            yypush_("Shifting", yyn, yyla);
            goto yynewstate;

        /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
        yydefault:
            yyn = yydefact_[yystack_[0].state];
            if (yyn == 0) goto yyerrlab;
            goto yyreduce;

        /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
        yyreduce:
            yylen = yyr2_[yyn];
            {
                stack_symbol_type yylhs;
                yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
                /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
                if (yylen)
                    yylhs.value = yystack_[yylen - 1].value;
                else
                    yylhs.value = yystack_[0].value;

                // Perform the reduction.
                YY_REDUCE_PRINT(yyn);
                try
                {
                    switch (yyn)
                    {
                    case 3:
#line 62 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node = node->parent();
                    }
#line 557 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 4:
#line 63 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node = node->parent();

                        // account for rogue characters found in gpg gas file
                        yyerrok;
                    }
#line 568 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 5:
#line 72 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node = node->appendChild((yystack_[0].value));
                    }
#line 574 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 6:
#line 73 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node = node->appendChild((yystack_[0].value), (yystack_[4].value));
                    }
#line 580 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 7:
#line 74 "src/FuelParser.y" // lalr1.cc:859
                    {

                        // TODO: do something with $1... like check for "dev"?
                        node = node->appendChild((yystack_[0].value), (yystack_[4].value));
                    }
#line 591 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 14:
#line 98 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node->appendValue((yystack_[3].value), (yystack_[1].value));
                    }
#line 597 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 15:
#line 99 "src/FuelParser.y" // lalr1.cc:859
                    {
                        node->appendValue((yystack_[3].value), (yystack_[4].value), (yystack_[1].value));
                    }
#line 603 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 18:
#line 108 "src/FuelParser.y" // lalr1.cc:859
                    {
                        (yylhs.value) = (yystack_[1].value) + (yystack_[0].value);
                    }
#line 609 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 20:
#line 113 "src/FuelParser.y" // lalr1.cc:859
                    {

                        (yylhs.value) = (yystack_[0].value);

                        (yylhs.value).erase(0, (yylhs.value).find_first_not_of(" \n\r\t"));
                        (yylhs.value).erase((yylhs.value).find_last_not_of(" \n\r\t") + 1);
                    }
#line 622 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

                    case 23:
#line 129 "src/FuelParser.y" // lalr1.cc:859
                    {
                        (yylhs.value) = (yystack_[2].value) + ':' + (yystack_[0].value);
                    }
#line 628 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    break;

#line 632 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:859
                    default: break;
                    }
                }
                catch (const syntax_error& yyexc)
                {
                    error(yyexc);
                    YYERROR;
                }
                YY_SYMBOL_PRINT("-> $$ =", yylhs);
                yypop_(yylen);
                yylen = 0;
                YY_STACK_PRINT();

                // Shift the result of the reduction.
                yypush_(YY_NULLPTR, yylhs);
            }
            goto yynewstate;

        /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
        yyerrlab:
            // If not already recovering from an error, report this error.
            if (!yyerrstatus_)
            {
                ++yynerrs_;
                error(yysyntax_error_(yystack_[0].state, yyla));
            }

            if (yyerrstatus_ == 3)
            {
                /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

                // Return failure if at end of input.
                if (yyla.type_get() == yyeof_)
                    YYABORT;
                else if (!yyla.empty())
                {
                    yy_destroy_("Error: discarding", yyla);
                    yyla.clear();
                }
            }

            // Else will try to reuse lookahead token after shifting the error token.
            goto yyerrlab1;

        /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
        yyerrorlab:

            /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
            if (false) goto yyerrorlab;
            /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
            yypop_(yylen);
            yylen = 0;
            goto yyerrlab1;

        /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
        yyerrlab1:
            yyerrstatus_ = 3; // Each real token shifted decrements this.
            {
                stack_symbol_type error_token;
                for (;;)
                {
                    yyn = yypact_[yystack_[0].state];
                    if (!yy_pact_value_is_default_(yyn))
                    {
                        yyn += yyterror_;
                        if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                        {
                            yyn = yytable_[yyn];
                            if (0 < yyn) break;
                        }
                    }

                    // Pop the current state because it cannot handle the error token.
                    if (yystack_.size() == 1) YYABORT;

                    yy_destroy_("Error: popping", yystack_[0]);
                    yypop_();
                    YY_STACK_PRINT();
                }

                // Shift the error token.
                error_token.state = yyn;
                yypush_("Shifting", error_token);
            }
            goto yynewstate;

            // Accept.
        yyacceptlab:
            yyresult = 0;
            goto yyreturn;

            // Abort.
        yyabortlab:
            yyresult = 1;
            goto yyreturn;

        yyreturn:
            if (!yyla.empty()) yy_destroy_("Cleanup: discarding lookahead", yyla);

            /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
            yypop_(yylen);
            while (1 < yystack_.size())
            {
                yy_destroy_("Cleanup: popping", yystack_[0]);
                yypop_();
            }

            return yyresult;
        }
        catch (...)
        {
            YYCDEBUG << "Exception caught: cleaning lookahead and stack" << std::endl;
            // Do not try to display the values of the reclaimed symbols,
            // as their printer might throw an exception.
            if (!yyla.empty()) yy_destroy_(YY_NULLPTR, yyla);

            while (1 < yystack_.size())
            {
                yy_destroy_(YY_NULLPTR, yystack_[0]);
                yypop_();
            }
            throw;
        }
    }

    void FuelParser ::error(const syntax_error& yyexc) { error(yyexc.what()); }

    // Generate an error message.
    std::string FuelParser ::yysyntax_error_(state_type, const symbol_type&) const { return YY_("syntax error"); }

    const signed char FuelParser ::yypact_ninf_ = -27;

    const signed char FuelParser ::yytable_ninf_ = -17;

    const signed char FuelParser ::yypact_[] = {10, -27, -3, 5, -27, -27, 10, -27, -27, -3, 6, -5, 15, 8, -27, -27, -27, 2, -3, 9, 1, -3, -3, 9, -27, -27,
                                                19, 11, 17, 10, 16, 18, 13, -27, -27, 10, 20, -3, -3, -27, 21, -27, 22, 23, -27, -3, -3, -27, 25, -3, -27};

    const unsigned char FuelParser ::yydefact_[] = {
        8, 21, 0, 0, 13, 2, 9, 10, 12, 0, 22, 0, 0, 5, 1, 11, 22, 0, 0, 19, 0, 0, 0, 19, 23, 17, 20, 0, 0, 8, 0, 0, 0, 18, 14, 8, 0, 0, 0, 15, 0, 3, 0, 0, 4, 0, 0, 6, 0, 0, 7};

    const signed char FuelParser ::yypgoto_[] = {-27, -27, -27, -27, -26, -27, 24, -27, -27, -27, 14, -2, 29};

    const signed char FuelParser ::yydefgoto_[] = {-1, 3, 4, 12, 5, 6, 7, 8, 9, 26, 27, 10, 11};

    const signed char FuelParser ::yytable_[] = {13, 1, 28, 36, 18, 14, 19, 16, 29, 40, -16, 18, 25, 23, 1, 2, 24, 21, 22, 30, 31, 20, 33, 34,
                                                 35, 39, 37, 38, 41, 44, 15, 45, 0, 46, 49, 42, 43, 32, 17, 0, 0, 0, 0, 47, 48, 0, 0, 50};

    const signed char FuelParser ::yycheck_[] = {2, 4, 1, 29, 9, 0, 11, 9, 7, 35, 4, 9, 3, 11, 4, 5, 18, 9, 10, 21, 22, 6, 3, 12, 7, 12, 10, 9, 8, 8, 6, 9, -1, 10, 9, 37, 38, 23, 9, -1, -1, -1, -1, 45, 46, -1, -1, 49};

    const unsigned char FuelParser ::yystos_[] = {0, 4, 5, 14, 15, 17, 18, 19, 20, 21, 24, 25, 16, 24, 0, 19, 24, 25, 9, 11, 6, 9, 10, 11, 24, 3,
                                                  22, 23, 1, 7, 24, 24, 23, 3, 12, 7, 17, 10, 9, 12, 17, 8, 24, 24, 8, 9, 10, 24, 24, 9, 24};

    const unsigned char FuelParser ::yyr1_[] = {0, 13, 14, 15, 15, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 25, 25};

    const unsigned char FuelParser ::yyr2_[] = {0, 2, 1, 6, 7, 1, 7, 9, 0, 1, 1, 2, 1, 1, 4, 5, 1, 1, 2, 0, 1, 1, 1, 3};

#if YYDEBUG
    // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
    // First, the terminals, then, starting at \a yyntokens_, nonterminals.
    const char* const FuelParser ::yytname_[] = {"$end",
                                                 "error",
                                                 "$undefined",
                                                 "\"expression\"",
                                                 "\"identifier\"",
                                                 "'['",
                                                 "']'",
                                                 "'{'",
                                                 "'}'",
                                                 "':'",
                                                 "','",
                                                 "'='",
                                                 "';'",
                                                 "$accept",
                                                 "translation_unit",
                                                 "element",
                                                 "element_name",
                                                 "element_body",
                                                 "element_body_list",
                                                 "element_body_item",
                                                 "attribute",
                                                 "type_id",
                                                 "expression_list",
                                                 "expression_statement",
                                                 "simple_identifier",
                                                 "identifier",
                                                 YY_NULLPTR};

    const unsigned char FuelParser ::yyrline_[] = {0, 58, 58, 62, 63, 72, 73, 74, 82, 84, 88, 89, 93, 94, 98, 99, 103, 107, 108, 111, 113, 124, 128, 129};

    // Print the state stack on the debug stream.
    void FuelParser ::yystack_print_()
    {
        *yycdebug_ << "Stack now";
        for (stack_type::const_iterator i = yystack_.begin(), i_end = yystack_.end(); i != i_end; ++i)
            *yycdebug_ << ' ' << i->state;
        *yycdebug_ << std::endl;
    }

    // Report on the debug stream that the rule \a yyrule is going to be reduced.
    void FuelParser ::yy_reduce_print_(int yyrule)
    {
        unsigned int yylno = yyrline_[yyrule];
        int yynrhs = yyr2_[yyrule];
        // Print the symbols being reduced, and their result.
        *yycdebug_ << "Reducing stack by rule " << yyrule - 1 << " (line " << yylno << "):" << std::endl;
        // The symbols being reduced.
        for (int yyi = 0; yyi < yynrhs; yyi++)
            YY_SYMBOL_PRINT("   $" << yyi + 1 << " =", yystack_[(yynrhs) - (yyi + 1)]);
    }
#endif // YYDEBUG

    // Symbol number corresponding to token number t.
    inline FuelParser ::token_number_type FuelParser ::yytranslate_(int t)
    {
        static const token_number_type translate_table[] = {
            0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 10, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 12, 2, 11, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 2, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 2, 8, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 3, 4};
        const unsigned int user_token_number_max_ = 259;
        const token_number_type undef_token_ = 2;

        if (static_cast<int>(t) <= yyeof_)
            return yyeof_;
        else if (static_cast<unsigned int>(t) <= user_token_number_max_)
            return translate_table[t];
        else
            return undef_token_;
    }

#line 24 "src/FuelParser.y" // lalr1.cc:1167
} // namespace ehb
#line 982 "E:/Programming/Projects/gitea/GameState/build/FuelParser.cpp" // lalr1.cc:1167
#line 132 "src/FuelParser.y"                                             // lalr1.cc:1168

#include <iostream>

namespace ehb
{
    void FuelParser::error(const std::string& msg)
    {
        // TODO: don't print this to cerr, but somewhere else user defined
        std::cerr << msg << std::endl;
    }
} // namespace ehb
