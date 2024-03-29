/*
History
=======
2009/09/05 Turley: Added struct .? and .- as shortcut for .exists() and .erase()

Notes
=======

*/

#ifndef __EXECUTOR_H
#define __EXECUTOR_H

#ifndef __SYMCONT_H
#include "symcont.h"
#endif

#ifndef __EXECTYPE_H
#include "exectype.h"
#endif

#ifndef __TOKEN_H
#include "token.h"
#endif

#ifndef __BOBJECT_H
#include "bobject.h"
#endif

#include "fmodule.h"
#include "eprog.h"

#include <stack>
#include <vector>
#include <exception>

class Executor;
class EScriptProgram;
class BLong;
class String;


// FIXME: how to make this a nested struct in Executor?
struct ReturnContext 
{
    unsigned PC;
    unsigned ValueStackDepth;
};
class Executor
{
  public:
    unsigned long sizeEstimate() const;

    friend void list_script( class UOExecutor* uoexec );
    int done;
    void seterror( bool err );
    bool error() const;
    bool error_;
    bool halt_;
    bool run_ok_;
    
    enum DEBUG_LEVEL { NONE, SOURCELINES, INSTRUCTIONS };
    DEBUG_LEVEL debug_level; 
    unsigned PC; // program counter

	bool AttachFunctionalityModules();
    

    bool setProgram( EScriptProgram* prog );

    BObjectRefVec Globals2;

    stack<BObjectRefVec*> upperLocals2;

    stack<ReturnContext> ControlStack;

    BObjectRefVec* Locals2;

    static UninitObject* m_SharedUninitObject;

    typedef stack< BObjectRef,vector<BObjectRef> > ValueStackCont;
	ValueStackCont ValueStack;

	static ExecInstrFunc GetInstrFunc( const Token& token );

/*
	These must both be deleted.  instr references _symbols, so it should be deleted first. 
	FIXME: there should be a separate object, called EProgram or something,
	that owns both the instructions and the symbols.  It should be ref_counted,
	so a code repository can store programs that multiple Executors use.
	That means debugger stuff has to come out of Instruction.
*/
    unsigned nLines;

    vector<BObjectRef> fparams; 

    friend class ExecutorModule;
    void setFunctionResult( BObjectImp* imp );

  protected:
    int getParams(unsigned howMany);
    void cleanParams();
   
  public:
    int makeString(unsigned param);
    bool hasParams(unsigned howmany) const { return (fparams.size() >= howmany); }
    unsigned numParams() const { return fparams.size(); }
    BObjectImp* getParamImp(unsigned param);
    BObjectImp* getParamImp(unsigned param, BObjectImp::BObjectType type);
    BObjectImp* getParamImp2(unsigned param, BObjectImp::BObjectType type);
    BObject* getParamObj( unsigned param );

	const String* getStringParam(unsigned param);
    const BLong* getLongParam(unsigned param);

    bool getStringParam( unsigned param, const String*& pstr );
    bool getParam( unsigned param, long& value );
    bool getParam( unsigned param, long& value, long maxval );
    bool getParam( unsigned param, long& value, long minval, long maxval );
    bool getRealParam( unsigned param, double& value );
    bool getObjArrayParam( unsigned param, ObjArray*& pobjarr );

    bool getParam( unsigned param, unsigned& value );

	bool getParam( unsigned param, unsigned short& value );
    bool getParam( unsigned param, unsigned short& value, unsigned short maxval );
    bool getParam( unsigned param, unsigned short& value, unsigned short minval, unsigned short maxval );

	bool getParam( unsigned param, short& value );
	bool getParam( unsigned param, short& value, short maxval );
	bool getParam( unsigned param, short& value, short minval, short maxval );
    
	void* getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type );
	BApplicObjBase* getApplicObjParam( unsigned param, const BApplicObjType* object_type );
   
    
    const char* paramAsString(unsigned param);
    double paramAsDouble(unsigned param);
    long paramAsLong(unsigned param);
  
  protected:
    int makeDouble(unsigned param);
    

    BObject* getParam(unsigned param);

    void check_containers(void);
    BObject getValue(void);
    BObjectRef getObjRef(void);

  public:
    int getToken(Token& token, unsigned position);
    BObjectRef& LocalVar( unsigned long varnum );
    BObjectRef& GlobalVar( unsigned long varnum );
    BObject* makeObj( const Token& token);
	int makeGlobal( const Token& token );
	void popParam( const Token& token );
    void popParamByRef( const Token& token );
    void getArg( const Token& token );
    void pushArg( BObjectImp* arg );
    void pushArg( const BObjectRef& ref );

    BObjectRef addmember( BObject& left, const BObject& right );
	BObjectRef removemember( BObject& left, const BObject& right);
	BObjectRef checkmember( BObject& left, const BObject& right);
    void addmember2( BObject& left, const BObject& right );

    // execmodules: modules associated with the current program.  References modules owned by availmodules.
    vector<ExecutorModule*> execmodules;
    vector<ExecutorModule*> availmodules; // owns
    
  public:
    explicit Executor( ostream& cerr );
    virtual ~Executor();

    void addModule(ExecutorModule* module); // NOTE, executor deletes its modules when done
    ExecutorModule* findModule( const string& name );

    ModuleFunction* current_module_function;
    // NOTE: the debugger code expects these to be virtual..
    void execFunc(const Token& token);
    void innerExec(const Instruction& ins);
    void execInstr();

    void ins_nop( const Instruction& ins );
    void ins_jmpiftrue( const Instruction& ins );
    void ins_jmpiffalse( const Instruction& ins );
    void ins_globalvar( const Instruction& ins );
    void ins_localvar( const Instruction& ins );
    void ins_makeLocal( const Instruction& ins );
    void ins_declareArray( const Instruction& ins );
    void ins_long( const Instruction& ins );
    void ins_double( const Instruction& ins );
    void ins_string( const Instruction& ins );
    void ins_error( const Instruction& ins );
    void ins_struct( const Instruction& ins );
    void ins_array( const Instruction& ins );
    void ins_dictionary( const Instruction& ins );
    void ins_uninit( const Instruction& ins );
    void ins_ident( const Instruction& ins );
    void ins_unminus( const Instruction& ins );
    
    void ins_logical_and( const Instruction& ins );
    void ins_logical_or( const Instruction& ins );
    void ins_logical_not( const Instruction& ins );
    
    void ins_bitwise_not( const Instruction& ins );

    void ins_set_member( const Instruction& ins );
    void ins_set_member_consume( const Instruction& ins );
    void ins_get_member( const Instruction& ins );
    void ins_get_member_id( const Instruction& ins ); //test id
    void ins_set_member_id( const Instruction& ins ); //test id
    void ins_set_member_id_consume( const Instruction& ins ); //test id
    
    void ins_assign_localvar( const Instruction& ins );
    void ins_assign_globalvar( const Instruction& ins );
    void ins_assign_consume( const Instruction& ins );
    void ins_consume( const Instruction& ins );
    void ins_assign( const Instruction& ins );
    void ins_array_assign( const Instruction& ins );
    void ins_array_assign_consume( const Instruction& ins );
    void ins_multisubscript_assign( const Instruction& ins );
    void ins_multisubscript_assign_consume( const Instruction& ins );
    void ins_multisubscript( const Instruction& ins );
    
    void ins_add( const Instruction& ins );
    void ins_subtract( const Instruction& ins );
    void ins_mult( const Instruction& ins );
    void ins_div( const Instruction& ins );
    void ins_modulus( const Instruction& ins );

	void ins_insert_into( const Instruction& ins );

	void ins_plusequal( const Instruction& ins );
	void ins_minusequal( const Instruction& ins );
	void ins_timesequal( const Instruction& ins );
	void ins_divideequal( const Instruction& ins );
	void ins_modulusequal( const Instruction& ins );

	void ins_bitshift_right( const Instruction& ins );
	void ins_bitshift_left( const Instruction& ins );
    void ins_bitwise_and( const Instruction& ins );
    void ins_bitwise_xor( const Instruction& ins );
    void ins_bitwise_or( const Instruction& ins );

    void ins_equal( const Instruction& ins );
    void ins_notequal( const Instruction& ins );
    void ins_lessthan( const Instruction& ins );
    void ins_lessequal( const Instruction& ins );
    void ins_greaterthan( const Instruction& ins );
    void ins_greaterequal( const Instruction& ins );

    void ins_goto( const Instruction& ins );
    void ins_arraysubscript( const Instruction& ins );
    void ins_func( const Instruction& ins );
    void ins_call_method( const Instruction& ins );
    void ins_call_method_id( const Instruction& ins );
    void ins_statementbegin( const Instruction& ins );
    void ins_progend( const Instruction& ins );
    void ins_makelocal( const Instruction& ins );
    void ins_jsr_userfunc( const Instruction& ins );
    void ins_pop_param( const Instruction& ins );
    void ins_pop_param_byref( const Instruction& ins );
    void ins_get_arg( const Instruction& ins );
    void ins_leave_block( const Instruction& ins );
    void ins_gosub( const Instruction& ins );
    void ins_return( const Instruction& ins );
    void ins_exit( const Instruction& ins );
    
    void ins_member( const Instruction& ins );
    void ins_addmember( const Instruction& ins );
	void ins_removemember( const Instruction& ins );
	void ins_checkmember( const Instruction& ins );
    void ins_dictionary_addmember( const Instruction& ins );
    void ins_addmember2( const Instruction& ins );
    void ins_addmember_assign( const Instruction& ins );
    void ins_in( const Instruction& ins );

    void ins_initforeach( const Instruction& ins );
    void ins_stepforeach( const Instruction& ins );
    void ins_initforeach2( const Instruction& ins );
    void ins_stepforeach2( const Instruction& ins );

    void ins_casejmp( const Instruction& ins );
    void ins_initfor( const Instruction& ins );
    void ins_nextfor( const Instruction& ins );

    int ins_casejmp_findlong( const Token& token, BLong* blong );
    int ins_casejmp_findstring( const Token& token, String* bstringimp );
    int ins_casejmp_finddefault( const Token& token );
    
    
    bool runnable() const;
    void calcrunnable();
    
    bool halt() const;
    void sethalt( bool halt );

    bool debugging() const;
    void setdebugging( bool debugging );

    void attach_debugger();
    void detach_debugger();
    string dbg_get_instruction( unsigned atPC ) const;
    void dbg_ins_trace();
    void dbg_step_into();
    void dbg_step_over();
    void dbg_run();
    void dbg_break();
    void dbg_setbp( unsigned atPC );
    void dbg_clrbp( unsigned atPC );
    void dbg_clrallbp();

    bool exec();
    void reinitExec();
    void initForFnCall( unsigned in_PC );
    void show_context( unsigned atPC );
    void show_context( ostream& os, unsigned atPC );

	long getDebugLevel( ) { return debug_level; }
    void setDebugLevel( DEBUG_LEVEL level ) { debug_level = level; }
    void setViewMode( bool vm ) { viewmode_ = vm; }
    const string& scriptname() const;
    bool empty_scriptname();
    const EScriptProgram* prog() const;

  private:
    ref_ptr<EScriptProgram> prog_;
    bool prog_ok_;
    bool viewmode_;

    bool debugging_;
    enum DEBUG_STATE { DEBUG_STATE_NONE, 
                       DEBUG_STATE_ATTACHING,
                       DEBUG_STATE_ATTACHED,
                       DEBUG_STATE_INS_TRACE,
                       DEBUG_STATE_INS_TRACE_BRK,
                       DEBUG_STATE_RUN,
                       DEBUG_STATE_BREAK_INTO,
                       DEBUG_STATE_STEP_INTO,
                       DEBUG_STATE_STEPPING_INTO,
                       DEBUG_STATE_STEP_OVER,
                       DEBUG_STATE_STEPPING_OVER };
    DEBUG_STATE debug_state_; 
    set<unsigned> breakpoints_;
    set<unsigned> tmpbreakpoints_;
    unsigned bp_skip_;

    BObjectImp* func_result_;

  private: // not implemented
    Executor( const Executor& exec );
    Executor& operator=( const Executor& exec );
};

inline const string& Executor::scriptname() const
{
	return prog_->name;
}

inline bool Executor::empty_scriptname()
{
	return prog_->name.empty();
}

inline const EScriptProgram* Executor::prog() const
{
    return prog_.get();
}

inline bool Executor::runnable(void) const
{
    return run_ok_;
}
inline void Executor::calcrunnable()
{
    run_ok_ = !error_ && !halt_;
}

inline void Executor::seterror( bool err )
{
    error_ = err;
    calcrunnable();
}
inline bool Executor::error() const
{
    return error_;
}

inline void Executor::sethalt( bool halt )
{
    halt_ = halt;
    calcrunnable();
}
inline bool Executor::halt() const
{
    return halt_;
}

inline bool Executor::debugging() const
{
    return debugging_;
}
inline void Executor::setdebugging( bool debugging )
{
    debugging_ = debugging;
}

#endif
