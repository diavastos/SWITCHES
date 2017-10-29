/**************************************************/
/*                                                */
/*  File:        switches.y                       */
/*  Description: Contains the pragma directives   */
/*                                                */
/*  Author:      Andreas I. Diavastos             */
/*  Contact:     diavastos@cs.ucy.ac.cy           */
/*  Last Update: 01-07-2017                       */
/*                                                */
/**************************************************/

%{
    #include "definitions.h"
    
    
    /* What we need from flex */
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
    
    /* Translator Global Variables */	
	int  line 				= 1;
	int  pass 				= 1;
    int  currentFile 		= 0;
    int  currentFunction 	= 1;
    int  currentTask 		= 1;
    int  currentFor			= -1;
    int  targetSystem 		= 0;
    int  runtimeSystem 		= RUNTIME_STATIC;
    int  printSGFlag 		= 0;
    int  assignmentPolicy 	= SCHED_RR;
    int  affinityPolicy     = AFFINITY_NONE;
   
    SG*         Graph;
    
    /* NSGA Global Variables */
    NSGA*       nsga;
    Population* parent_pop;
    Population* child_pop;
    Population* mixed_pop;
    int         totalKernels = 0;
    double*     min_var;
    double*     max_var;
    
    bool firstPass 		= TRUE;
    bool inPragmaBlock 	= FALSE;
    bool inParallelFor  = FALSE;
    bool inReduction	= FALSE;

    bool transactions 	= FALSE;
    
    
	FILE *inp, *outp_sw_main, *outp_sw_h, *outp_sw_threadpool, *outp_sw_threads, *outp_sw_tao_h;
	FILE *outp;
	
	// Local variables
	int localSimd 				= 0;
	int localKernels 			= 0;
	int localScheduling 		= 0;
	int localPriority 			= 0;
	int localStateOfDefault 	= 0;
	int localDependenceType		= 0;
	
	intStr 	localintStr;
	
	char	**stringFor;
	char	reductionType[REDUCTION_SIZE];
	
	
	dataList 		*lists[NUM_LISTS];
	arrayIndex    	*indexes;
    
    extern bool inPragmaLine;
	extern bool inPragmaFor;
	extern bool inForLine;
    extern bool bracketsEnabled;
    extern bool inMainFunction;
    extern int 	inWhichDataList;
    extern int  bracketCounter;
    
    
    
    extern int 	kernels;
    extern char **inputFiles;
    extern int 	totalInputFiles;
    
    
	void yyerror(const char *s);
%}


 /* ANSI C Language Tokens -- 2011 ISO C Standard Compatible */

%token IDENTIFIER STRING_LITERAL F_CONSTANT I_CONSTANT
 
%token C_DEFAULT C_STATIC _ALIGNOF _ATOMIC _STATIC_ASSERT AUTO FUNC_NAME 
%token CHAR CONST DOUBLE FLOAT INT LONG SHORT BOOL SIGNED SIZEOF STRUCT UNION
%token UNSIGNED _BOOL _COMPLEX _IMAGINARY VOID RESTRICT VOLATILE _GENERIC

%token LE_OPERAND GE_OPERAND EQ_OPERAND NE_OPERAND
%token RIGHT_ASSIGN LEFT_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN
%token DIV_ASSIGN MOD_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN RIGHT_OPERAND
%token LEFT_OPERAND INC_OPERAND DEC_OPERAND PTR_OPERAND AND_OPERAND OR_OPERAND



 /* OpenMP 4.5 Token Definiton */

%token PRAGMA OMP PARALLEL FOR SECTIONS SECTION SIMD TASK SINGLE TARGET ATOMIC
%token MASTER CRITICAL BARRIER NUM_THREADS DEFAULT PRIVATE FIRSTPRIVATE SHARED
%token COPYIN REDUCTION NOWAIT DEPEND LASTPRIVATE COPYPRIVATE LINEAR COLLAPSE
%token IF FINAL UNTIED MERGEABLE PRIORITY SCHEDULE ORDERED GRAINSIZE NUM_TASKS
%token NOGROUP ALIGNED SAFELEN DECLARE SIMDLEN UNIFORM INBRANCH NOTINBRANCH 
%token TASKLOOP UPDATE DISTRIBUTE DEVICE TO FROM TASKYIELD DATA ENTER EXIT MAP
%token IS_DEVICE_PTR DEFAULTMAP NUM_TEAMS THREAD_LIMIT DIST_SCHEDULE TEAMS
%token TASKWAIT TASKGROUP READ WRITE CAPTURE FLUSH THREADS CANCEL CANCELLATION 
%token POINT THREADPRIVATE IN OUT INOUT NONE PROC_BIND CLOSE SPREAD
%token MAX MIN SEQ_CST ALLOC TOFROM USE_DEVICE_PTR SCALAR
%token GUIDED DYNAMIC STATIC RUNTIME CROSS

/* My Token Definitions */

%token ENDLN GLOBAL



%start program

%%

program:	omp_statement
	|		program omp_statement
	;

omp_statement:	omp_construct
	|			omp_directive
	;

primary_expression:     IDENTIFIER
                |       constant
                |       string
                |       '(' expression ')'
                |       generic_selection
				;

constant:   I_CONSTANT		        /* includes character_constant */
	|       F_CONSTANT
	;

string:     STRING_LITERAL
    |       FUNC_NAME
	;

generic_selection: _GENERIC '(' assignment_expression ',' generic_assoc_list ')'
	;

generic_assoc_list:     generic_association
    |                   generic_assoc_list ',' generic_association
	;

generic_association:    type_name ':' assignment_expression
	|                   C_DEFAULT ':' assignment_expression
	;

postfix_expression:     primary_expression
	|                   postfix_expression '[' expression ']'
	|                   postfix_expression '(' ')'
	|                   postfix_expression '(' argument_expression_list ')'
	|                   postfix_expression '.' IDENTIFIER
	|                   postfix_expression PTR_OPERAND IDENTIFIER
	|                   postfix_expression INC_OPERAND
	|                   postfix_expression DEC_OPERAND
	|                   '(' type_name ')' '{' initializer_list '}'
	|                   '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list:   assignment_expression
	|                       argument_expression_list ',' assignment_expression
	;

unary_expression:   postfix_expression
	|               INC_OPERAND unary_expression
	|               DEC_OPERAND unary_expression
	|               unary_operator cast_expression
	|               SIZEOF unary_expression
	|               SIZEOF '(' type_name ')'
	|               _ALIGNOF '(' type_name ')'
	;

unary_operator:     '&'
	|               '*'
	|               '+'
	|               '-'
	|               '~'
	|               '!'
	;

cast_expression:    unary_expression
	|               '(' type_name ')' cast_expression
	;

multiplicative_expression:  cast_expression
	|                       multiplicative_expression '*' cast_expression
	|                       multiplicative_expression '/' cast_expression
	|                       multiplicative_expression '%' cast_expression
	;

additive_expression:    multiplicative_expression
	|                   additive_expression '+' multiplicative_expression
	|                   additive_expression '-' multiplicative_expression
	;

shift_expression:   additive_expression
	|               shift_expression LEFT_OPERAND additive_expression
	|               shift_expression RIGHT_OPERAND additive_expression
	;

relational_expression:  shift_expression
	|                   relational_expression '<' shift_expression
	|                   relational_expression '>' shift_expression
	|                   relational_expression LE_OPERAND shift_expression
	|                   relational_expression GE_OPERAND shift_expression
	;

equality_expression:    relational_expression
	|                   equality_expression EQ_OPERAND relational_expression
	|                   equality_expression NE_OPERAND relational_expression
	;

and_expression:     equality_expression
	|               and_expression '&' equality_expression
	;

exclusive_or_expression:    and_expression
	|                       exclusive_or_expression '^' and_expression
	;

inclusive_or_expression:    exclusive_or_expression
	|                       inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression:     inclusive_or_expression
	|                       logical_and_expression AND_OPERAND inclusive_or_expression
	;

logical_or_expression:  logical_and_expression
	|                   logical_or_expression OR_OPERAND logical_and_expression
	;

conditional_expression:     logical_or_expression
	|                       logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression:  conditional_expression
	|                   unary_expression assignment_operator assignment_expression
	;

assignment_operator:    '='
	|                   MUL_ASSIGN
	|                   DIV_ASSIGN
	|                   MOD_ASSIGN
	|                   ADD_ASSIGN
	|                   SUB_ASSIGN
	|                   LEFT_ASSIGN
	|                   RIGHT_ASSIGN
	|                   AND_ASSIGN
	|                   XOR_ASSIGN
	|                   OR_ASSIGN
	;

expression:     assignment_expression
	|           expression ',' assignment_expression
	;

constant_expression: conditional_expression
	;

type_specifier:     VOID
	|               CHAR
	|               SHORT
	|               INT
	|               LONG
	|               FLOAT
	|               DOUBLE
	|				BOOL
	|               SIGNED
	|               UNSIGNED
	|               _BOOL
	|               _COMPLEX
	|               _IMAGINARY
	|               atomic_type_specifier
	|               struct_or_union_specifier
	;

struct_or_union_specifier:  struct_or_union '{' struct_declaration_list '}'
	|                       struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	|                       struct_or_union IDENTIFIER
	;

struct_or_union:    STRUCT
	|               UNION
	;

struct_declaration_list:    struct_declaration
	|                       struct_declaration_list struct_declaration
	;

struct_declaration:     specifier_qualifier_list ';'
	|                   specifier_qualifier_list struct_declarator_list ';'
	|                   static_assert_declaration
	;

specifier_qualifier_list:   type_specifier specifier_qualifier_list
	|                       type_specifier
	|                       type_qualifier specifier_qualifier_list
	|                       type_qualifier
	;

struct_declarator_list:     struct_declarator
	|                       struct_declarator_list ',' struct_declarator
	;

struct_declarator:  ':' constant_expression
	|               declarator ':' constant_expression
	|               declarator
	;

atomic_type_specifier:  _ATOMIC '(' type_name ')'		/* Creates a shift/reduce conflict that is resolved by default */
	;

type_qualifier:     CONST
	|               RESTRICT
	|               VOLATILE
	|               _ATOMIC
	;

declarator:     pointer direct_declarator
	|           direct_declarator
	;

direct_declarator:  IDENTIFIER
	|               '(' declarator ')'
	|               direct_declarator '[' ']'
	|               direct_declarator '[' '*' ']'
	|               direct_declarator '[' C_STATIC type_qualifier_list assignment_expression ']'
	|               direct_declarator '[' C_STATIC assignment_expression ']'
	|               direct_declarator '[' type_qualifier_list '*' ']'
	|               direct_declarator '[' type_qualifier_list C_STATIC assignment_expression ']'
	|               direct_declarator '[' type_qualifier_list assignment_expression ']'
	|               direct_declarator '[' type_qualifier_list ']'
	|               direct_declarator '[' assignment_expression ']'
//	|               direct_declarator '(' parameter_type_list ')'
	|               direct_declarator '(' ')'
	|               direct_declarator '(' identifier_list ')'
	;

pointer:    '*' type_qualifier_list pointer
	|       '*' type_qualifier_list
	|       '*' pointer
	|       '*'
	;

type_qualifier_list:    type_qualifier
	|                   type_qualifier_list type_qualifier
	;


identifier_list:    IDENTIFIER
	|               identifier_list ',' IDENTIFIER
	;

type_name:  specifier_qualifier_list abstract_declarator
	|       specifier_qualifier_list
	;

abstract_declarator:    pointer direct_abstract_declarator
	|                   pointer
	|                   direct_abstract_declarator
	;

direct_abstract_declarator:     '(' abstract_declarator ')'
	|                           '[' ']'
	|                           '[' '*' ']'
	|                           '[' C_STATIC type_qualifier_list assignment_expression ']'
	|                           '[' C_STATIC assignment_expression ']'
	|                           '[' type_qualifier_list C_STATIC assignment_expression ']'
	|                           '[' type_qualifier_list assignment_expression ']'
	|                           '[' type_qualifier_list ']'
	|                           '[' assignment_expression ']'
	|                           direct_abstract_declarator '[' ']'
	|                           direct_abstract_declarator '[' '*' ']'
	|                           direct_abstract_declarator '[' C_STATIC type_qualifier_list assignment_expression ']'
	|                           direct_abstract_declarator '[' C_STATIC assignment_expression ']'
	|                           direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
	|                           direct_abstract_declarator '[' type_qualifier_list C_STATIC assignment_expression ']'
	|                           direct_abstract_declarator '[' type_qualifier_list ']'
	|                           direct_abstract_declarator '[' assignment_expression ']'
	|                           '(' ')'
//	|                           '(' parameter_type_list ')'
	|                           direct_abstract_declarator '(' ')'
//	|                           direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer:    '{' initializer_list '}'
	|           '{' initializer_list ',' '}'
	|           assignment_expression
	;

initializer_list:   designation initializer
	|               initializer
	|               initializer_list ',' designation initializer
	|               initializer_list ',' initializer
	;

designation:    designator_list '='
	;

designator_list:    designator
	|               designator_list designator
	;

designator:     '[' constant_expression ']'
	|           '.' IDENTIFIER
	;

static_assert_declaration:  _STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
	;


expression_statement:   ';'
	|                   expression ';'
	;
	
iteration_statement_for:
		FOR
		{
			if(!firstPass)
				WRITE("%s", "    for");
				
		}
/*	|	FOR '(' ';' ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    for ( ; ; )");
			
			inForLine = FALSE;
		}
	| 	FOR '(' expression ';' ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' expression ';' ';' ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' ';' expression ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' ';' expression ';' ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' ';' ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' ';' ';' expression ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' expression ';' expression ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' expression ';' expression ';' ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' expression ';' ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' expression ';' ';' expression ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' ';' expression ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' ';' expression ';' expression ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' expression ';' expression ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' expression ';' expression ';' expression ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' declaration ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' declaration ';' ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' declaration expression ';' ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' declaration expression ';' ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' declaration ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' declaration ';' expression ')'");
				
			inForLine = FALSE;
		}
	| 	FOR '(' declaration expression ';' expression ')'
		{
			if(!firstPass)
				WRITE("%s", "    FOR '(' declaration expression ';' expression ')'");
				
			inForLine = FALSE;
		}*/
	;
	
	
/*
 * --> NEEDED BY ITERATION FOR STATEMENT IF WE ARE GOING OT USE BISON TO RECOGNIZE THE PARTS OF THE FOR STATEMENT
 * 
 * 
declaration:
		declaration_specifiers ';'
		{
		
		}
	| 	declaration_specifiers init_declarator_list ';'
		{
		
		}
	;
	
declaration_specifiers:
		type_specifier
	    {
	      
	    }
	| 	type_specifier declaration_specifiers
	    {
	      
	    }
	;
	
init_declarator_list:
		init_declarator
	    {
			
	    }
	| 	init_declarator_list ',' init_declarator
	    {

	    }
	;
	
init_declarator:
	    IDENTIFIER
	    {
	      
	    }
	| 	IDENTIFIER '='
	    {
	    
	    }
		initializer
		{
		
		}
	;
*/


 /* OpenMP v4.5 Bison rules */
 
omp_construct:
        parallel_construct
    |   for_construct
    |   sections_construct
    |   single_construct
    |	section_construct
    |   parallel_for_construct
    |   parallel_sections_construct
    |   master_construct
    |   critical_construct
    |   atomic_construct
    |   ordered_construct
    |   task_construct
    |   simd_construct
    |   for_simd_construct
    |   parallel_for_simd_construct
    |   target_data_construct
    |   target_construct
    |   target_simd_construct
    |   target_parallel_construct
    |   target_parallel_for_construct
    |   target_parallel_for_simd_construct
    |   target_update_construct
    |   teams_construct
    |   distribute_construct
    |   distribute_simd_construct
    |   distribute_parallel_for_construct
    |   distribute_parallel_for_simd_construct
    |   target_teams_construct
    |   teams_distribute_construct
    |   teams_distribute_simd_construct
    |   target_teams_distribute_construct
    |   target_teams_distribute_simd_construct
    |   teams_distribute_parallel_for_construct
    |   target_teams_distribute_parallel_for_construct
    |   teams_distribute_parallel_for_simd_construct
    |   target_teams_distribute_parallel_for_simd_construct
    |   taskgroup_construct
    |	declare_simd_construct
    |	taskloop_construct
    |	taskloop_simd_construct
    ;

omp_directive:
        omp_barrier
    |   omp_flush
    |   omp_taskwait
    |   omp_taskyield
    |   omp_cancel
    |   omp_cancellation_point
	|	omp_target_enter_data
	|	omp_target_exit_data
	|	omp_threadprivate
	|	omp_declare_reduction
    ;

optional_expression:
		':' expression
	|
	;


 /************ Directives & Construct definitions ************/



 /* TARGET SIMD Construct */
 
target_simd_construct:
		target_simd_directive iteration_statement_for
		{
			
		}
	;
	
target_simd_directive:
		PRAGMA OMP TARGET SIMD target_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target simd ]\n");
			
			inReduction = FALSE;
			line++;
			
			
		}
	;
	
target_simd_clause_multi:
		target_simd_clause_multi target_simd_clause
	|	target_simd_clause_multi ',' target_simd_clause
	|
	;
	
target_simd_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	last_private_clause
	|	reduction_clause
	|	collapse_clause	
	|	NOWAIT
	;
	
 
 
 /* TARGET PARALLEL FOR SIMD Construct */
 
target_parallel_for_simd_construct:
		target_parallel_for_simd_directive iteration_statement_for
		{
			
		}
	;
	
target_parallel_for_simd_directive:
		PRAGMA OMP TARGET PARALLEL FOR SIMD target_parallel_for_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target parallel for simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;
	
target_parallel_for_simd_clause_multi:
		target_parallel_for_simd_clause_multi target_parallel_for_simd_clause
	|	target_parallel_for_simd_clause_multi ',' target_parallel_for_simd_clause
	|
	;
 
target_parallel_for_simd_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_threads_clause
	|   procbind_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause	
	|	ordered_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause	
	|	last_private_clause	
	|	NOWAIT
	;
 
 
 
 /* TARGET PARALLEL FOR Construct */

target_parallel_for_construct:
		target_parallel_for_directive iteration_statement_for
		{
			
		}
	;
	
target_parallel_for_directive:
		PRAGMA OMP TARGET PARALLEL FOR target_parallel_for_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target parallel for ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;
	
target_parallel_for_clause_multi:
		target_parallel_for_clause_multi target_parallel_for_clause
	|	target_parallel_for_clause_multi ',' target_parallel_for_clause
	|
	;
		
target_parallel_for_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_threads_clause
	|   procbind_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause
	|	ordered_clause
	|	schedule_clause
	|	collapse_clause
	|	last_private_clause	
	|	NOWAIT
	;
	
 
 
 /* TARGET PARALLEL Construct */
 
target_parallel_construct:
		target_parallel_directive //structured_block
	;
	
target_parallel_directive:
		PRAGMA OMP TARGET PARALLEL target_parallel_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target parallel  ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;
	
target_parallel_clause_multi:
		target_parallel_clause_multi target_parallel_clause
	|	target_parallel_clause_multi ',' target_parallel_clause
	|
	;

target_parallel_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_threads_clause
	|   procbind_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause
	|	NOWAIT
	;		
		
 
 
 /* TASKLOOP SIMD Construct */

taskloop_simd_construct:
		taskloop_simd_directive iteration_statement_for
		{
			
		}
	;
	
taskloop_simd_directive:
		PRAGMA OMP TASKLOOP SIMD taskloop_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ taskloop simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;
	
	
taskloop_simd_clause_multi:
		taskloop_simd_clause_multi taskloop_simd_clause
	|	taskloop_simd_clause_multi ',' taskloop_simd_clause
	|
	;

taskloop_simd_clause:
		if_clause
	|	shared_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	default_clause
	|	grainsize_clause
	|	num_tasks_clause
	|	collapse_clause
	|	final_clause
	|	priority_clause
	|	untied_clause
	|	mergeable_clause
	|	NOGROUP
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	reduction_clause	
	;
	
 
 
 /* TASKLOOP Construct */
 
taskloop_construct:
		taskloop_directive iteration_statement_for
		{
			
		}
	;

taskloop_directive:
		PRAGMA OMP TASKLOOP taskloop_clause_multi ENDLN
		{
			if(inReduction)
				handlePragma_taskConstruct(&Graph, lists, TASK_REDUCTION, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			else
				handlePragma_taskConstruct(&Graph, lists, TASK_LOOP, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localKernels = 0;
			localPriority = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localSimd = 0;
			inReduction = FALSE;
			currentTask++;
			line++;			
		}
	;

taskloop_clause_multi:
		taskloop_clause_multi taskloop_clause
	|	taskloop_clause_multi ',' taskloop_clause
	|
	;

taskloop_clause:
		if_clause
	|	shared_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	default_clause
	|	grainsize_clause
	|	num_tasks_clause
	|	collapse_clause
	|	final_clause
	|	priority_clause
	|	untied_clause
	|	mergeable_clause
	|	NOGROUP
    // SWITCHES EXTENSIONS
    |	num_threads_clause
	|	schedule_clause
	|	reduction_clause
	|	depend_clause
	;


 
 /* DECLARE SIMD Construct */
 
declare_simd_construct:
		declare_simd_directive //function_def
	//|	declare_simd_directive declaration
	;
	
declare_simd_directive:
		PRAGMA OMP DECLARE SIMD declare_simd_directive_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ declare simd ]\n");
			line++;
			
		}
	;
	
declare_simd_directive_clause_multi:
		declare_simd_directive_clause_multi declare_simd_directive_clause
	|	declare_simd_directive_clause_multi ',' declare_simd_directive_clause
	|
	;

declare_simd_directive_clause:
		simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	uniform_clause
	|	INBRANCH
	|	NOTINBRANCH
	;
	

 
 /* TASKGROUP Construct */
 
taskgroup_construct: 
		taskgroup_directive //structured_block
	;

taskgroup_directive:
		PRAGMA OMP TASKGROUP ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ taskgroup ]\n");
			line++;
			
		}
	;
 
 
 /* TARGET TEAMS DISTRIBUTE PARALLEL FOR SIMD Construct */
 
target_teams_distribute_parallel_for_simd_construct:
		target_teams_distribute_parallel_for_simd_directive iteration_statement_for
		{
			
		}
	;

target_teams_distribute_parallel_for_simd_directive:
		PRAGMA OMP TARGET TEAMS DISTRIBUTE PARALLEL FOR SIMD target_teams_distribute_parallel_for_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target teams distribute parallel for simd ]\n");
			
			inReduction = FALSE;
			line++;
			
		}
	;

target_teams_distribute_parallel_for_simd_clause_multi:
		target_teams_distribute_parallel_for_simd_clause_multi target_teams_distribute_parallel_for_simd_clause
	|	target_teams_distribute_parallel_for_simd_clause_multi ',' target_teams_distribute_parallel_for_simd_clause
	|
	;

target_teams_distribute_parallel_for_simd_clause:
		dist_schedule_clause
	|	if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause	
	|	ordered_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause	
	|	last_private_clause	
	|	device_clause
	|	map_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_teams_clause
	|	thread_limit_clause
	|	NOWAIT
	;

 
 
 /* TEAMS DISTRIBUTE PARALLEL FOR SIMD Construct */

teams_distribute_parallel_for_simd_construct:
		teams_distribute_parallel_for_simd_directive iteration_statement_for
		{
			
		}
	;

teams_distribute_parallel_for_simd_directive:
		PRAGMA OMP TEAMS DISTRIBUTE PARALLEL FOR SIMD teams_distribute_parallel_for_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ teams distribute parallel for simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

teams_distribute_parallel_for_simd_clause_multi:
		teams_distribute_parallel_for_simd_clause_multi teams_distribute_parallel_for_simd_clause
	|	teams_distribute_parallel_for_simd_clause_multi ',' teams_distribute_parallel_for_simd_clause
	|
	;

teams_distribute_parallel_for_simd_clause:
		dist_schedule_clause
	|	if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause	
	|	ordered_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause	
	|	last_private_clause	
	|	device_clause
	|	map_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_teams_clause
	|	thread_limit_clause
	|	NOWAIT
	;
 
 
 
 /* TARGET TEAMS DISTRIBUTE PARALLEL FOR Construct */

target_teams_distribute_parallel_for_construct:
		target_teams_distribute_parallel_for_directive iteration_statement_for
		{
			
		}
	;

target_teams_distribute_parallel_for_directive:
		PRAGMA OMP TARGET TEAMS DISTRIBUTE PARALLEL FOR target_teams_distribute_parallel_for_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target teams distribute parallel for ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

target_teams_distribute_parallel_for_clause_multi:
		target_teams_distribute_parallel_for_clause_multi target_teams_distribute_parallel_for_clause
	|	target_teams_distribute_parallel_for_clause_multi ',' target_teams_distribute_parallel_for_clause
	|
	;

target_teams_distribute_parallel_for_clause:
		num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	if_clause
	|   copyin_clause
	|   procbind_clause
	|	ordered_clause
	|	schedule_clause
	|	collapse_clause
	|	last_private_clause
	|	dist_schedule_clause	
	|	device_clause
	|	map_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	NOWAIT
	;



 /* TEAMS DISTRIBUTE PARALLEL FOR Construct */
 
teams_distribute_parallel_for_construct:
		teams_distribute_parallel_for_directive iteration_statement_for
		{
			
		}
	;

teams_distribute_parallel_for_directive:
		PRAGMA OMP TEAMS DISTRIBUTE PARALLEL FOR teams_distribute_parallel_for_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ teams distribute parallel for ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

teams_distribute_parallel_for_clause_multi:
		teams_distribute_parallel_for_clause_multi teams_distribute_parallel_for_clause
	|	teams_distribute_parallel_for_clause_multi ',' teams_distribute_parallel_for_clause
	|
	;

teams_distribute_parallel_for_clause:
		num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	if_clause
	|   copyin_clause
	|   procbind_clause
	|	ordered_clause
	|	schedule_clause
	|	collapse_clause
	|	last_private_clause
	|	dist_schedule_clause	
	; 

 
 
 /* TARGET TEAMS DISTRIBUTE SIMD Construct */
 
target_teams_distribute_simd_construct:
		target_teams_distribute_simd_directive iteration_statement_for
		{
			
		}
    ;

target_teams_distribute_simd_directive:
		PRAGMA OMP TARGET TEAMS DISTRIBUTE SIMD target_teams_distribute_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target teams distribute simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

target_teams_distribute_simd_clause_multi:
		target_teams_distribute_simd_clause_multi target_teams_distribute_simd_clause
	|	target_teams_distribute_simd_clause_multi ',' target_teams_distribute_simd_clause
    |
    ;

target_teams_distribute_simd_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause
	|	dist_schedule_clause
	|	last_private_clause
	|	collapse_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	NOWAIT
	;

 
 
 /* TARGET TEAMS DISTRIBUTE Construct */
 
target_teams_distribute_construct:
		target_teams_distribute_directive iteration_statement_for
		{
			
		}
;

target_teams_distribute_directive:
		PRAGMA OMP TARGET TEAMS DISTRIBUTE target_teams_distribute_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target teams distribute ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

target_teams_distribute_clause_multi:
		target_teams_distribute_clause_multi target_teams_distribute_clause
	|	target_teams_distribute_clause_multi ',' target_teams_distribute_clause
	|
	;

target_teams_distribute_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause
	|	dist_schedule_clause
	|	last_private_clause
	|	collapse_clause
	|	NOWAIT
	;
 
 
 
 /* TEAMS DISTRIBUTE SIMD Construct */
 
teams_distribute_simd_construct:
		teams_distribute_simd_directive iteration_statement_for
		{
			
		}
	;

teams_distribute_simd_directive:
		PRAGMA OMP TEAMS DISTRIBUTE SIMD teams_distribute_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ teams distribute simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

teams_distribute_simd_clause_multi:
		teams_distribute_simd_clause_multi teams_distribute_simd_clause
	|	teams_distribute_simd_clause_multi ',' teams_distribute_simd_clause
	|
	;

teams_distribute_simd_clause:
		num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	dist_schedule_clause
	|	last_private_clause
	|	collapse_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	;



 /* TEAMS DISTRIBUTE Construct */

teams_distribute_construct:
		teams_distribute_directive iteration_statement_for
		{
			
		}
    ;

teams_distribute_directive:
		PRAGMA OMP TEAMS DISTRIBUTE teams_distribute_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ teams distribute ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

teams_distribute_clause_multi:
		teams_distribute_clause_multi teams_distribute_clause
	|	teams_distribute_clause_multi ',' teams_distribute_clause
	|
	;

teams_distribute_clause:
		num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	dist_schedule_clause
	|	last_private_clause
	|	collapse_clause
	;

 
 
 /* TARGET TEAMS Construct */
 
target_teams_construct:
		target_teams_directive //structured_block
	;

target_teams_directive:
		PRAGMA OMP TARGET TEAMS target_teams_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target teams ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

target_teams_clause_multi:
		target_teams_clause_multi target_teams_clause
	|	target_teams_clause_multi ',' target_teams_clause
	|
	;

target_teams_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	shared_clause
	|	reduction_clause
	|	NOWAIT
	;
 
 
 
 /* DISTRIBUTE PARALLEL FOR SIMD Construct */
 
distribute_parallel_for_simd_construct:
		distribute_parallel_for_simd_directive //structured_block
	;

distribute_parallel_for_simd_directive:
		PRAGMA OMP DISTRIBUTE PARALLEL FOR SIMD distribute_parallel_for_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ distribute parallel for simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

distribute_parallel_for_simd_clause_multi:
		distribute_parallel_for_simd_clause_multi distribute_parallel_for_simd_clause
	|	distribute_parallel_for_simd_clause_multi ',' distribute_parallel_for_simd_clause
	|
	;

distribute_parallel_for_simd_clause:
		dist_schedule_clause
	|	if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause	
	|	ordered_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause	
	|	last_private_clause	
	;
 
 

 /* DISTRIBUTE PARALLEL FOR Construct */
 
distribute_parallel_for_construct:
		distribute_parallel_for_directive iteration_statement_for
		{
			
		}    
	;

distribute_parallel_for_directive:
		PRAGMA OMP DISTRIBUTE PARALLEL FOR distribute_parallel_for_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ distribute parallel for ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

distribute_parallel_for_clause_multi:
		distribute_parallel_for_clause_multi distribute_parallel_for_clause
	|	distribute_parallel_for_clause_multi ',' distribute_parallel_for_clause
	|
	;

distribute_parallel_for_clause:
		if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	ordered_clause
	|	schedule_clause
	|	collapse_clause
	|	last_private_clause
	|	dist_schedule_clause
	;
 
 
 
 /* DISTRIBUTE SIMD Construct */
 
distribute_simd_construct:
		distribute_simd_directive iteration_statement_for
		{
			
		}
	;

distribute_simd_directive:
		PRAGMA OMP DISTRIBUTE SIMD distribute_simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ distribute simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

distribute_simd_clause_multi:
		distribute_simd_clause_multi distribute_simd_clause
	|	distribute_simd_clause_multi ',' distribute_simd_clause
	|
	;

distribute_simd_clause:
		dist_schedule_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	collapse_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	reduction_clause
	;
	
 
 
 /* DISTRIBUTE Construct */

distribute_construct:
		distribute_directive iteration_statement_for
		{
			
		}
	;

distribute_directive:
		PRAGMA OMP DISTRIBUTE distribute_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ distribute ]\n");
			line++;
			
		}
	;

distribute_clause_multi:
		distribute_clause_multi distribute_clause
	|	distribute_clause_multi ',' distribute_clause
	|
	;

distribute_clause:
		dist_schedule_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	collapse_clause
	;

 
 
 /* TEAMS Construct */

teams_construct:
		teams_directive //structured_block
	;

teams_directive:
		PRAGMA OMP TEAMS teams_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ teams ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

teams_clause_multi:
		teams_clause_multi teams_clause
	|	teams_clause_multi ',' teams_clause
	|
	;

teams_clause:
		num_teams_clause
	|	thread_limit_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	;

 
 
 /* TARGET UPDATE Construct */
 
target_update_construct:
		target_update_directive
	;

target_update_directive:
		PRAGMA OMP TARGET UPDATE target_update_clause_seq ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target update ]\n");
			line++;
			
		}
	;

target_update_clause_seq:
		target_update_clause
	|	target_update_clause_seq target_update_clause
	|	target_update_clause_seq ',' target_update_clause
	;

target_update_clause:
		motion_clause
	|	device_clause
	|	if_clause
	|	depend_clause
	| 	NOWAIT
	;
 
 
 
 /* TARGET Construct */
 
target_construct:
		target_directive //structured_block
	;

target_directive:
		PRAGMA OMP TARGET target_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target ]\n");
			line++;
			
		}
	;

target_clause_multi:
		target_clause_multi target_clause
	|	target_clause_multi ',' target_clause
	|
	;

target_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	private_clause
	|	first_private_clause
	|	IS_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	|	DEFAULTMAP '(' TOFROM ':' SCALAR ')'
	|	depend_clause
	|	NOWAIT
	;



 /* TARGET DATA Construct */
 
target_data_construct:
		target_data_directive //structured_block
    ;

target_data_directive:
		PRAGMA OMP TARGET DATA target_data_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ target data ]\n");
			line++;
			
		}
	;

target_data_clause_multi:
		target_data_clause_multi target_data_clause
	|	target_data_clause_multi ',' target_data_clause
	|
	;

target_data_clause:
		device_clause
	|	map_clause
	|	if_clause
	|	USE_DEVICE_PTR '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;
 
 
 
 /* PARALLEL FOR SIMD Construct */
 
parallel_for_simd_construct:
		parallel_for_simd_directive iteration_statement_for
		{
			
		}
	;

parallel_for_simd_directive:
		PRAGMA OMP PARALLEL FOR SIMD parallel_for_simd_clause_multi ENDLN
		{
			handlePragma_parallelConstruct(&Graph, localKernels, localStateOfDefault, lists, currentFunction);
			if(inReduction)
				handlePragma_taskConstruct(&Graph, lists, TASK_REDUCTION, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction, currentTask);
			else
				handlePragma_taskConstruct(&Graph, lists, TASK_LOOP, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction, currentTask);
			
			if(!firstPass)
			{
				bracketsEnabled = TRUE;
				inParallelFor = TRUE;
			}
			localKernels = 0;
			localPriority = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localSimd = 0;
			inReduction = FALSE;
			currentFunction++;
			currentTask++;
			line++;
			
		}
	;

parallel_for_simd_clause_multi:
		parallel_for_simd_clause_multi parallel_for_simd_clause
	|	parallel_for_simd_clause_multi ',' parallel_for_simd_clause
	|
	;

parallel_for_simd_clause:
		if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause	
	|	ordered_clause
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause	
	|	last_private_clause
	;
  

 
 /* FOR SIMD Construct */
 
for_simd_construct:
		for_simd_directive iteration_statement_for
		{
			
		}
	;

for_simd_directive:
		PRAGMA OMP FOR SIMD for_simd_clause_multi ENDLN
		{
			if(inReduction)
				handlePragma_taskConstruct(&Graph, lists, TASK_REDUCTION, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			else
				handlePragma_taskConstruct(&Graph, lists, TASK_LOOP, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localKernels = 0;
			localPriority = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localSimd = 0;
			inReduction = FALSE;
			currentTask++;
			line++;
			
		}
	;

for_simd_clause_multi:
		for_simd_clause_multi for_simd_clause
	|	for_simd_clause_multi ',' for_simd_clause
	|
	;

for_simd_clause:
		ordered_clause
	|	num_threads_clause		
	|	safe_len_clause
	|	simd_len_clause
	|	linear_clause
	|	aligned_clause
	|	schedule_clause
	|	collapse_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	reduction_clause
	|	NOWAIT
	|	depend_clause
	;


 
 /* SIMD Construct */
 
simd_construct:
		simd_directive iteration_statement_for
		{
			
		}
	;


simd_directive:
		PRAGMA OMP SIMD simd_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ simd ]\n");
				
			inReduction = FALSE;
			line++;
			
		}
	;

simd_clause_multi:
		simd_clause_multi simd_clause
	|	simd_clause_multi ',' simd_clause
	|
	;

simd_clause:
		safe_len_clause
	|	simd_len_clause
	|	private_clause
	|	linear_clause
	|	aligned_clause
	|	last_private_clause
	|	reduction_clause
	|	collapse_clause
	;
	
	
 
 /* TASK Construct */
 
task_construct:
		task_directive //structured_block
	;

task_directive:
		PRAGMA OMP TASK task_clause_multi ENDLN
		{
			handlePragma_taskConstruct(&Graph, lists, TASK_SIMPLE, 1, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localPriority = 0;
			localSimd = 0;
			currentTask++;
			line++;
		}
	;

task_clause_multi:
		task_clause_multi task_clause
	|	task_clause_multi ',' task_clause
	|
	;

task_clause:
		if_clause
	|	final_clause
	|	untied_clause
	|	mergeable_clause
	| 	depend_clause
	|	priority_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	;


 
 /* ORDERED Construct */
 
ordered_construct:
		ordered_directive //structured_block
	;

ordered_directive:
		PRAGMA OMP ORDERED ordered_clause_multi ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ ordered ]\n");
			line++;
			
		}
	;

ordered_clause_multi:
		ordered_clause_multi ordered_clause_opt
	|	ordered_clause_multi ',' ordered_clause_opt
	|
	;
 
 
 
 /* ATOMIC Construct */
 
atomic_construct:
		atomic_directive expression_statement
	;

atomic_directive:
		PRAGMA OMP ATOMIC atomic_clause seq_cst_clause ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ atomic ]\n");
			line++;
			
		}
	;


 
 /* CRITICAL Construct */

critical_construct:
		critical_directive //structured_block
	;

critical_directive:
		PRAGMA OMP CRITICAL ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ critical ]\n");
			line++;
		}
	|	PRAGMA OMP CRITICAL region_phrase ENDLN
		{
			if(!firstPass)
				WRITE("%s", "FOUND PRAGMA [ critical (...) ]\n");
			line++;
		}
	;
 
 
 
 /* MASTER Construct */

master_construct:
		master_directive //structured_block
	;

master_directive:
		PRAGMA OMP MASTER ENDLN
		{
			handlePragma_taskConstruct(&Graph, lists, TASK_MASTER, 1, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localPriority = 0;
			localSimd = 0;
			currentTask++;
			line++;
		}
	;


 
 /* PARALLEL SECTIONS Construct */
 
parallel_sections_construct:
		parallel_sections_directive
	;

parallel_sections_directive:
		PRAGMA OMP PARALLEL SECTIONS parallel_sections_clause_multi ENDLN
		{
			handlePragma_parallelConstruct(&Graph, localKernels, localStateOfDefault, lists, currentFunction);
			handlePragma_sectionsConstruct(&Graph, lists);
			if(!firstPass)
				bracketsEnabled = TRUE;
			localKernels = 0;
			localStateOfDefault = 0;
			inReduction = FALSE;
			currentFunction++;
			line++;
			
		}
	;

parallel_sections_clause_multi:
		parallel_sections_clause_multi parallel_sections_clause
	|	parallel_sections_clause_multi ',' parallel_sections_clause
	|
	;

parallel_sections_clause:
		if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	shared_clause
	|	reduction_clause
	;



 /* PARALLEL FOR Construct */

parallel_for_construct:
		parallel_for_directive iteration_statement_for
		{
			
		}
	;

parallel_for_directive:
		PRAGMA OMP PARALLEL FOR parallel_for_clause_multi ENDLN
		{
			handlePragma_parallelConstruct(&Graph, localKernels, localStateOfDefault, lists, currentFunction);
			
			if(inReduction)
				handlePragma_taskConstruct(&Graph, lists, TASK_REDUCTION, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction, currentTask);
			else
				handlePragma_taskConstruct(&Graph, lists, TASK_LOOP, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction, currentTask);
			
			if(!firstPass)
			{
				bracketsEnabled = TRUE;
				inParallelFor = TRUE;
			}
			localKernels = 0;
			localPriority = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localSimd = 0;
			inReduction = FALSE;
			currentFunction++;
			currentTask++;
			line++;
			
		}
	;

parallel_for_clause_multi:
		parallel_for_clause_multi parallel_for_clause
	|	parallel_for_clause_multi ',' parallel_for_clause
	|
	;

parallel_for_clause:
		if_clause
	|	num_threads_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	|	ordered_clause
	|	schedule_clause
	|	collapse_clause
	|	last_private_clause
	;
 
 
 
 /* SINGLE Construct */
 
single_construct:
		single_directive //structured_block
	;

single_directive:
		PRAGMA OMP SINGLE single_clause_multi ENDLN
		{
			handlePragma_taskConstruct(&Graph, lists, TASK_SIMPLE, 1, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localPriority = 0;
			localSimd = 0;
			currentTask++;
			line++;
		}
	;

single_clause_multi:
		single_clause_multi single_clause
	|	single_clause_multi ',' single_clause
    |
	;

single_clause:
		copy_private_clause
	|	private_clause
	|	first_private_clause
	|	NOWAIT
	;

 
 
 /* SECTIONS Construct */

sections_construct:
		sections_directive
	;

sections_directive:
		PRAGMA OMP SECTIONS sections_clause_multi ENDLN
		{
			handlePragma_sectionsConstruct(&Graph, lists);
			if(!firstPass)
				bracketsEnabled = TRUE;
				
			inReduction = FALSE;
			line++;	
		}
	;

sections_clause_multi:
		sections_clause_multi sections_clause
	|	sections_clause_multi ',' sections_clause
    |
	;

sections_clause:
		private_clause
	|	first_private_clause
	|	last_private_clause
	|	reduction_clause
	|	NOWAIT
	;


 /* SECTION Construct */

section_construct:
		section_directive
	;

section_directive:
		PRAGMA OMP SECTION ENDLN
		{
			handlePragma_sectionDirective(&Graph, lists, currentFunction -1, currentTask);
			if(!firstPass)
				bracketsEnabled = TRUE;
			currentTask++;
			line++;
			
		}
	;
	
	

 /* FOR Construct */
 
for_construct:
		for_directive iteration_statement_for
	;

for_directive:
		PRAGMA OMP FOR for_clause_multi ENDLN
		{
			if(inReduction)
				handlePragma_taskConstruct(&Graph, lists, TASK_REDUCTION, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			else
				handlePragma_taskConstruct(&Graph, lists, TASK_LOOP, localKernels, localStateOfDefault, localPriority, localScheduling, localintStr, localSimd, currentFunction-1, currentTask);
			
			if(!firstPass)
				bracketsEnabled = TRUE;
			localDependenceType = 0;
			localKernels = 0;
			localPriority = 0;
			localStateOfDefault = 0;
			localScheduling = 0;
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
			localSimd = 0;
			inReduction = FALSE;
			currentTask++;
			line++;
			
		}
	;

for_clause_multi:
		for_clause_multi for_clause
	|	for_clause_multi ',' for_clause
	|
	;

for_clause:
		ordered_clause
	|	num_threads_clause
	|	schedule_clause
	|	collapse_clause
	|	private_clause
	|	first_private_clause
	|	last_private_clause
	|	reduction_clause
	|	NOWAIT
	|	depend_clause
	;



 /* PARALLEL Construct */
 
parallel_construct:
		parallel_directive
    ;

parallel_directive:
		PRAGMA OMP PARALLEL parallel_clause_multi ENDLN
		{
			handlePragma_parallelConstruct(&Graph, localKernels, localStateOfDefault, lists, currentFunction);
			
			if(!firstPass)
				bracketsEnabled = TRUE;
			localKernels = 0;
			localStateOfDefault = 0;
			inReduction = FALSE;
			currentFunction++;
			line++;
			
			
		}
	;

parallel_clause_multi:
		parallel_clause_multi parallel_clause
	|	parallel_clause_multi ',' parallel_clause
	|
	;

parallel_clause:
		if_clause
	|	num_threads_clause
    |	depend_clause
	|   copyin_clause
	|   procbind_clause
	|	default_clause
	|	private_clause
	|	first_private_clause
	|	shared_clause
	|	reduction_clause
	;
	


 /* DECLARE Directive */
 
omp_declare_reduction:
	PRAGMA OMP DECLARE reduction_clause ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ declare reduction ]\n");
        line++;

    }
    ;
 
 
 /* THREADPRIVATE Directive */

omp_threadprivate:
	PRAGMA OMP THREADPRIVATE '(' variable_list ')' ENDLN
	{
        handlePragma_threadPrivateDirective(&Graph, lists);    
        line++;
        inWhichDataList = 0;
    }
    ;
 
 
 
 /* BARRIER Directive */
 
omp_barrier:
    PRAGMA OMP BARRIER ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ barrier ]\n");
        line++;
    }
    ;



 /* FLUSH Directive */
 
omp_flush:
    PRAGMA OMP FLUSH ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ flush ]\n");
        line++;
    }
    |
    PRAGMA OMP FLUSH flush_list ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ flush ]\n");
        line++;

    }
    ;

flush_list:
    '(' variable_list ')'
    {
		inWhichDataList = 0;
    }
    ;



 /* TASKWAIT Directive */
 
omp_taskwait:
    PRAGMA OMP TASKWAIT ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ taskwait ]\n");
        line++;
    }
    ;



 /* TASKYIELD Directive */
 
omp_taskyield:
    PRAGMA OMP TASKYIELD ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ taskyield ]\n");
        line++;
    }
    ;


/* TARGET ENTER DATA Directive */
 
omp_target_enter_data:
	PRAGMA OMP TARGET ENTER DATA omp_target_enter_data_clause_multi ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ target enter data ]\n");
        line++;

    }
    ;

omp_target_enter_data_clause_multi:
		omp_target_enter_data_clause_multi omp_target_enter_data_clause
	|	omp_target_enter_data_clause_multi ',' omp_target_enter_data_clause
	|
	;
	
omp_target_enter_data_clause:
		if_clause
	|	device_clause
	|	map_clause
	|	depend_clause
	|	NOWAIT
	;
	
	
/* TARGET EXIT DATA Directive */
 
omp_target_exit_data:
	PRAGMA OMP TARGET EXIT DATA omp_target_exit_data_clause_multi ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ target exit data ]\n");
        line++;

    }
    ;

omp_target_exit_data_clause_multi:
		omp_target_exit_data_clause_multi omp_target_exit_data_clause
	|	omp_target_exit_data_clause_multi ',' omp_target_exit_data_clause
	|
	;
	
omp_target_exit_data_clause:
		if_clause
	|	device_clause
	|	map_clause
	|	depend_clause
	|	NOWAIT
	;


 /* CANCEL Directive */
 
omp_cancel:
    PRAGMA OMP CANCEL type_construct_clause ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ cancel ]\n");
        line++;

    }
    |
    PRAGMA OMP CANCEL type_construct_clause if_clause ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ cancel ]\n");
        line++;

    }
    |
    PRAGMA OMP CANCEL type_construct_clause ',' if_clause ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ cancel ]\n");
        line++;

    }
    ;
    
    

 /* CANCELLATION_POINT Directive */
 
omp_cancellation_point:
    PRAGMA OMP CANCELLATION POINT type_construct_clause ENDLN
	{
        if(!firstPass)
			WRITE("%s", "FOUND PRAGMA [ cancellation point ]\n");
        line++;

    }
    ;


 /*** General rules that apply to multiple pragma directives -- clauses ***/


grainsize_clause:
		GRAINSIZE '(' I_CONSTANT ')'
	;
	
num_tasks_clause:
		NUM_TASKS '(' I_CONSTANT ')'
	;

uniform_clause:
		UNIFORM '(' argument_expression_list ')'
	;

dist_schedule_clause:
		DIST_SCHEDULE '(' STATIC ')'
	|	DIST_SCHEDULE '(' STATIC ',' expression ')'
	;

num_teams_clause:
		NUM_TEAMS '(' expression ')'
	;

thread_limit_clause:
	   THREAD_LIMIT '(' expression ')'
	;

motion_clause:
		TO '(' variable_array_section_list ')'
	|	FROM '(' variable_array_section_list ')'
	;

device_clause:
		DEVICE '(' expression ')'
    ;

map_clause:
		MAP '(' map_type ':' variable_array_section_list ')'
	|	MAP '(' variable_array_section_list ')'
	;

map_type:
		ALLOC
	|	TO
	|	FROM
	|	TOFROM
	;

safe_len_clause:
		SAFELEN '(' expression ')'
	;
	
simd_len_clause:
		//SIMDLEN '(' expression ')'
		SIMDLEN '(' I_CONSTANT ')'
		{
			localSimd = $3;
		}
	;

final_clause:
		FINAL '(' expression ')'
	;

untied_clause:
		UNTIED
	;

mergeable_clause:
		MERGEABLE
	;

depend_clause:
		DEPEND '(' dependence_type ':' variable_array_section_list ')'
		{
			
			inWhichDataList = 0;
		}
	;

dependence_type:
		IN			{ inWhichDataList = IN_DEPEND_IN; 	 }
	|	OUT			{ inWhichDataList = IN_DEPEND_OUT;	 }
	|	INOUT		{ inWhichDataList = IN_DEPEND_INOUT; }
	|				{ yyerror("Dependence type not recognized"); }
	;

priority_clause:
		//PRIORITY '(' expression ')'
		PRIORITY '(' I_CONSTANT ')'
		{
			localPriority = $3;
		}
	;

ordered_clause_opt:
		THREADS
	|	SIMD
	;

atomic_clause:
		READ
	|	WRITE
	|	UPDATE
	|	CAPTURE
	|
	;

seq_cst_clause:
		SEQ_CST
	|
	;

region_phrase:
		'(' IDENTIFIER ')'    
	;

copy_private_clause:
		COPYPRIVATE '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;

ordered_clause:
		ORDERED
	|	ORDERED '(' expression ')'
	
schedule_clause:
		SCHEDULE '(' schedule_kind ')'
		{
			localintStr.localInt = 0;
			localintStr.localStr = NULL;
		}
	|	SCHEDULE '(' schedule_kind ',' I_CONSTANT ')'
		{
			localintStr.localInt = $5;
		}
	|	SCHEDULE '(' schedule_kind ',' IDENTIFIER ')'
		{	
			localintStr.localStr = (char *)malloc(sizeof(char)*strlen((char*)$5));
			strcpy(localintStr.localStr, (char*)$5);
		}
	//|	SCHEDULE '(' schedule_kind ',' expression ')'
	;

schedule_kind:
		STATIC		{ localScheduling = LOOP_SCHED_STATIC;  }
	|	CROSS		{ localScheduling = LOOP_SCHED_CROSS;   }
	|				{ yyerror("Scheduling policy not recognized"); }
	;

num_threads_clause:
		NUM_THREADS '(' I_CONSTANT ')'
		//NUM_THREADS '(' expression ')'
		{
			localKernels = $3;
		}
	;
	
copyin_clause:
		COPYIN '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;

default_clause:
		DEFAULT '(' SHARED ')'
		{
			localStateOfDefault = DEFAULT_SHARED;
		}
	|	DEFAULT '(' NONE ')'
		{
			localStateOfDefault = DEFAULT_NONE;
		}
	;

private_clause:
		PRIVATE '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;
	
first_private_clause:
		FIRSTPRIVATE '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;

last_private_clause:
		LASTPRIVATE '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;

shared_clause:
		SHARED '(' variable_list ')'
		{
			inWhichDataList = 0;
		}
	;

reduction_clause:
		REDUCTION '(' reduction_identifier ':' variable_list ')'
		{
			bzero(reductionType, sizeof(reductionType));
			inWhichDataList = 0;
		}
	;

linear_clause:
		LINEAR '(' variable_list optional_expression ')'
		{
			inWhichDataList = 0;
		}
	;


aligned_clause:
		ALIGNED '(' variable_list optional_expression ')'
		{
			inWhichDataList = 0;
		}
	;


variable_array_section_list:
		IDENTIFIER 
		{
			handlePragma_variableList(inWhichDataList, lists, (char *)$1, NULL, reductionType, NULL);
		}
	|	array_section
	|	variable_array_section_list ',' IDENTIFIER
		{
			handlePragma_variableList(inWhichDataList, lists, (char *)$3, NULL, reductionType, NULL);
		
		}
	|	variable_array_section_list ',' array_section
	;

array_section:
		IDENTIFIER array_section_subscript
		{
			handlePragma_variableList(inWhichDataList, lists, (char *)$1, NULL, reductionType, &indexes);
		}
	;

array_section_subscript:
		array_section_subscript '[' array_section_plain ']'
	|	'[' array_section_plain ']'
	;

array_section_plain:
		//expression ':' expression
	//|	expression
		IDENTIFIER
		{
			if(firstPass)
				addToIndexes(&indexes, -1, -1, (char*)$1, NULL);
		}
	|	I_CONSTANT
		{
			if(firstPass)
				addToIndexes(&indexes, $1, -1, NULL, NULL);
		}
	|	IDENTIFIER ':' IDENTIFIER
		{
			if(firstPass)
				addToIndexes(&indexes, -1, -1, (char*)$1, (char*)$3);
		}
	|	IDENTIFIER ':' I_CONSTANT
		{
			if(firstPass)
				addToIndexes(&indexes, -1, $3, (char*)$1, NULL);
		}
	|	I_CONSTANT ':' IDENTIFIER
		{
			if(firstPass)
				addToIndexes(&indexes, $1, -1, NULL, (char*)$3);
		}
	|	I_CONSTANT ':' I_CONSTANT
		{
			if(firstPass)
				addToIndexes(&indexes, $1, $3, NULL, NULL);
		}
	;

collapse_clause:
		COLLAPSE '(' expression ')'
	;

variable_list:
    variable_list ',' IDENTIFIER
    {
		handlePragma_variableList(inWhichDataList, lists, (char *)$3, NULL, reductionType, NULL);
    }
    |
    IDENTIFIER
    {
		handlePragma_variableList(inWhichDataList, lists, (char *)$1, NULL, reductionType, NULL);
    }
    |
    I_CONSTANT
    {
		handlePragma_variableList(inWhichDataList, lists, (char *)$1, NULL, reductionType, NULL);
    }
    ;

type_construct_clause:
		PARALLEL
    |   SECTIONS
    |   FOR
    |   TASKGROUP
    |	{ yyerror("Construct type not recognized"); }
    ;
  
if_clause:
    IF '(' expression ')'
    ;
  
procbind_clause:
		PROC_BIND '(' MASTER ')'
	|	PROC_BIND '(' CLOSE ')'
	|	PROC_BIND '(' SPREAD ')'
    ;
    
reduction_identifier:
		IDENTIFIER
		{
			strcpy(reductionType, (char*)$1);
		}
	|	'+'
		{
			strcpy(reductionType, "+");
		}
	|	'*'
		{
			strcpy(reductionType, "*");
		}
	|	'-'
		{
			strcpy(reductionType, "-");
		}
	|	'&'
		{
			strcpy(reductionType, "&");
		}
	|	'^'
		{
			strcpy(reductionType, "^");
		}
	|	'|'
		{
			strcpy(reductionType, "|");
		}
	|	AND_OPERAND
		{
			strcpy(reductionType, "&&");
		}
	|	OR_OPERAND
		{
			strcpy(reductionType, "||");
		}
	|	MIN
		{
			strcpy(reductionType, "min");
		}
	| 	MAX
		{
			strcpy(reductionType, "max");
		}
	;
 
%%

int main(int argc, char *argv[]){
    
    int i = 0, j = 0, k = 0, dum;
    char outputFile[INPUT_SIZE];
        
    
    /*** Recognize & store command line arguments in global variables ***/
    recognizeCommandlineArguments(argc, argv);
    
    /*** Initialize the SG ***/
    addSynchronizationGraph(&Graph);
    
    /*** Initialize stringFor array ***/
    stringFor = (char**)malloc(sizeof(char*) * SIZE);
    for(i = 0; i < SIZE; i++)
		stringFor[i] = (char*)malloc(sizeof(char) * SIZE);
    
    for(i = 0; i < SIZE; i++)
		bzero(stringFor[i], sizeof(stringFor[i]));		
    
    /*** Temp descriptor to stdout ***/
    __OUTP_IS_NOW_STDOUT
   
   /*** Start multiple parsing of files ***/
   do
   { 
	    /*** Start parsing once all input files ***/
	    do
	    {	
			line = 1;
			        
	        // Open Input File
	        inp = fopen(inputFiles[currentFile], "r");
	        if(!inp){
	            ERROR_COMMANDS("File [ %s ] not found!", inputFiles[currentFile])
	            exit(-1);
	        }
	        
	        // Open Main Output File and start printing parallel source 
	        if(!firstPass)
	        {
				// Open [ sw_*.c ] Main Output Files
		        if(strlen(inputFiles[currentFile]) > INPUT_SIZE){
		            ERROR_COMMANDS("Input file name [ %s ] is too large", inputFiles[currentFile]);
		            exit(-1);
		        }
		        bzero(outputFile, sizeof(outputFile));
		        
                if(runtimeSystem == RUNTIME_TAO || runtimeSystem == RUNTIME_TAOSW){
                    sprintf(outputFile, "sw_%sxx", inputFiles[currentFile]);
                }
                else{
                    sprintf(outputFile, "sw_%s", inputFiles[currentFile]);
                }                
		        
		        outp_sw_main = fopen(outputFile, "w");
		        if(!outp_sw_main){
		            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
		            exit(-1);
		        }
		        __OUTP_IS_NOW_MAIN_FILE
                if(runtimeSystem == RUNTIME_STATIC){
                    WRITE("%s", "#include \"sw.h\"\n");
                }
                else{
                    WRITE("%s", "#include \"sw_tao.h\"\n");
                }
			}
	        
	        yyin = inp;         // Set input stream to be the input file
	        yyparse();          // Parse of input file
	        
	        fclose(inp);        		// Close input file
	        if(!firstPass)
			{
				__OUTP_IS_NOW_STDOUT
				fclose(outp_sw_main);       // Close output file
			}
				
	        // Move to next file
	        currentFile++;
	        bracketCounter = 0;
	        
	    }while(currentFile < totalInputFiles);
	       
	    
	    // Set Dependencies, Kernels to Functions, Scheduling, V-Kernels, etc.
		 
		if(firstPass)
		{
            // Define Assignment/Scheduling Policy here
            switch(assignmentPolicy){
                
                case SCHED_RR:
                                offlineScheduling_AssignKernelsToParallelFunctions(&Graph);
                                offlineScheduling_AssignKernelsToTasks_RoundRobin(&Graph);
                                break;
                                
                case SCHED_RANDOM:
                                offlineScheduling_AssignKernelsToParallelFunctions(&Graph);
                                offlineScheduling_AssignKernelsToTasks_Random(&Graph);
                                break;
                        
                case SCHED_FILE:
                                /* This assumes that the file is created by the GA
                                 *   - Kernels per parallel function are assumed to be the full number of the machine
                                 *   - If you need to load a scheduling policy from a file otherwise you must implement it here
                                 */
                                offlineScheduling_AssignKernelsToParallelFunctionsFromFile(&Graph);
                                offlineScheduling_AssignKernelsToTasks_File(&Graph);
                                break;
                                
                case SCHED_NSGA:
                                /* Assign Kernels to Parallel Functions and Count total kernels assigned */
                                totalKernels = offlineScheduling_CountKernelsToTasks_NSGA(&Graph);
                                
                                /* Allocate Memory for the parent, child and mixed populations */
                                allocatePopulation(&parent_pop, nsga->population);
                                allocatePopulation(&child_pop, nsga->population);
                                allocatePopulation(&mixed_pop, 2*(nsga->population));
                                
                                /* Randomly Initialize Generation 1 */
                                randomize();
                                NSGA_initializePopulation(&Graph, &parent_pop);
                                                                
                                /* Print Generation 1 to scheduling files and Evaluate Generation 1 */
                                NSGA_printPopulationToFiles(&parent_pop, 1);
                                NSGA_evaluatePopulation(&parent_pop, 1);
                                
                                /* Assign Rank and Crowding Distance */
                                NSGA_assign_rank_and_crowding_distance(&parent_pop);
                                
                                
                                /* Start Genetic Algorithm for all Generations */
                                for(i = 2; i <= nsga->generations; i++)
                                {
                                    /** Selection Process **/
                                    NSGA_selection(parent_pop, child_pop);
                                    
                                    /** Mutation Process **/
                                    NSGA_mutation(&child_pop);
                                    
                                    /** Print Generation to scheduling files and Evaluate Generation **/
                                    NSGA_printPopulationToFiles(&child_pop, i);
                                    NSGA_evaluatePopulation(&child_pop, i);
                                    
                                    /** Merge Parent with Child Generations **/
                                    NSGA_merge(&parent_pop, &child_pop, &mixed_pop);
                                    
                                    /** Fill Non-dominated Sorting **/
                                    NSGA_fill_nondominated_sort(&mixed_pop, &parent_pop);    
                                }

                                // End the process of the NSGA algorithm as soon as all generations are finished
                                exit(-1);
                                break;
                                
                default:
                    
                        ERROR_COMMANDS("Scheduling Policy [ %d ] not recognized!", assignmentPolicy)
                        exit(-1);
            }
            
			offlineScheduling_AssignVirtualKernelsToTasks(&Graph);			
			offlineScheduling_CreateDependencies(&Graph);
			offlineScheduling_TransitiveReductionOfConsumers(&Graph);
			offlineScheduling_TransitiveReductionOfProducers(&Graph);
		}
	    
	    
	    /*** Start printing in output files ***/
	    
	    if(firstPass)
	    {
            
            switch(runtimeSystem){
                
                case RUNTIME_STATIC:
                
                        // Open [ sw_threadpool.c ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_threadpool.c");
                        outp_sw_threadpool = fopen(outputFile, "w");
                        if(!outp_sw_threadpool){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw_threadpool.c ]
                        printInThreadpoolFile(&Graph);
                        
                        
                        // Open [ sw.h ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw.h");
                        outp_sw_h = fopen(outputFile, "w");
                        if(!outp_sw_h){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw.h ]
                        printInSwFile(&Graph);
                        
                        
                        // Open [ sw_threads.c ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_threads.c");
                        outp_sw_threads = fopen(outputFile, "w");
                        if(!outp_sw_threads){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        printInThreadsFile_headerSourceCode(&Graph);				// Print header source code of [ sw_threads.c ]
                        printInThreadsFile_SwitchesDeclaration(&Graph);				// Print switches in [ sw_threads.c ]
                        printInThreadsFile_taskCounters(&Graph);				    // Print taskCounters in [ sw_threads.c ]
                        printInThreadsFile_ResetSwitchesFunctions(&Graph);			// Print Reset Switches functions in [ sw_threads.c ]
                        printInThreadsFile_JobsThreadsFunction_SWITCHES(&Graph);	// Print Jobs Threads Function in [ sw_threads.c ]

                        break;
                        
                case RUNTIME_TAO:
                        
                        // Open [ sw.h ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw.h");
                        outp_sw_h = fopen(outputFile, "w");
                        if(!outp_sw_h){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw.h ]
                        printInSwFile(&Graph);
                        
                        
                        // Open [ sw_tao.h ] Output File -- TAO Classes
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_tao.h");
                        outp_sw_tao_h = fopen(outputFile, "w");
                        if(!outp_sw_tao_h){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw_tao.h ]
                        printInTaoFile(&Graph);
                        
                        
                        // Open [ sw_threads.c ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_threads.c");
                        outp_sw_threads = fopen(outputFile, "w");
                        if(!outp_sw_threads){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        printInThreadsFile_headerSourceCode(&Graph);				// Print header source code of [ sw_threads.c ]
                        printInThreadsFile_JobsThreadsFunction_TAO(&Graph);		    // Print Jobs Threads Function in [ sw_threads.c ]
                        break;
                        
                case RUNTIME_TAOSW:
                
                        // Open [ sw.h ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw.h");
                        outp_sw_h = fopen(outputFile, "w");
                        if(!outp_sw_h){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw.h ]
                        printInSwFile(&Graph);
                        
                        
                        // Open [ sw_tao.h ] Output File -- TAO Classes
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_tao.h");
                        outp_sw_tao_h = fopen(outputFile, "w");
                        if(!outp_sw_tao_h){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        // Print source code of [ sw_tao.h ]
                        printInTaoFile(&Graph);
                        
                        
                        // Open [ sw_threads.c ] Output File
                        bzero(outputFile, sizeof(outputFile));
                        sprintf(outputFile, "sw_threads.c");
                        outp_sw_threads = fopen(outputFile, "w");
                        if(!outp_sw_threads){
                            ERROR_COMMANDS("File [ %s ] not created!", outputFile)
                            exit(-1);
                        }
                        
                        printInThreadsFile_headerSourceCode(&Graph);				// Print header source code of [ sw_threads.c ]
                        printInThreadsFile_SwitchesDeclaration(&Graph);				// Print switches in [ sw_threads.c ]
                        printInThreadsFile_taskCounters(&Graph);				    // Print taskCounters in [ sw_threads.c ]
                        printInThreadsFile_ResetSwitchesFunctions(&Graph);			// Print Reset Switches functions in [ sw_threads.c ]
                        printInThreadsFile_JobsThreadsFunction_TAO(&Graph);		// Print Jobs Threads Function in [ sw_threads.c ]
                
                        break;
                
                default:
                    ERROR_COMMANDS("[ %s ] not recognized!", "Runtime System")
                    exit(-1);
            }
		}
		
		
		if(!firstPass)
		{
            switch(runtimeSystem){
                
                case RUNTIME_STATIC:
                        fclose(outp_sw_h);       	// Close output file [ sw.h ]
                        fclose(outp_sw_threads);    // Close output file [ sw_threads.c ]
                        fclose(outp_sw_threadpool); // Close output file [ sw_threadpool.c ]
                        break;
                        
                case RUNTIME_TAO:
                        fclose(outp_sw_h);       	// Close output file [ sw.h ]
                        fclose(outp_sw_threads);    // Close output file [ sw_threads.c ]
                        break;
                        
                case RUNTIME_TAOSW:
                        fclose(outp_sw_h);       	// Close output file [ sw.h ]
                        fclose(outp_sw_threads);    // Close output file [ sw_threads.c ]
                        break;
                
                default:
                    ERROR_COMMANDS("[ %s ] not recognized!", "Runtime System")
                    exit(-1);
            }
		}
			
		pass++;
	    firstPass = FALSE;
	    currentFile = 0;
	    currentTask = 1;
	    currentFor  = 0;
	    currentFunction = 1;
	    
    }while(pass <= PARSES);
    
    
    printSG(&Graph, printSGFlag);
    printErrorMessages(); 
    return 0;
    
}


void yyerror(const char *s){
	
    ERROR("[ %s ]", line, s)
    exit(-1);					/* Return when you find an error in a file.
								 * Because when you check the next file it 
								 * will always find an error even if there
								 * isn't one.
								 */
}

