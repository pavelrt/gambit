//#
//# FILE: gsm.cc  implementation of GSM (Stack machine)
//#
//# $Id$
//#


#include <assert.h>
#include "gambitio.h"


#include "gsm.h"


//--------------------------------------------------------------------
//              implementation of GSM (Stack machine)
//--------------------------------------------------------------------

GSM::GSM( int size, gOutput& s_out, gOutput& s_err )
:_StdOut( s_out ), _StdErr( s_err )
{
  
#ifndef NDEBUG
  if( size <= 0 )
    _ErrorMessage( _StdErr, 1, size );
#endif // NDEBUG
  
  _Stack         = new gGrowableStack< Portion* >( size );
  _CallFuncStack = new gGrowableStack< CallFuncObj* >( size ) ;
  _RefTable      = new RefHashTable;
  _FuncTable     = new FunctionHashTable;
  
  InitFunctions();  // This function is located in gsmfunc.cc
}


GSM::~GSM()
{
  Flush();
  delete _FuncTable;
  delete _RefTable;
  delete _CallFuncStack;
  delete _Stack;
}


int GSM::Depth( void ) const
{
  return _Stack->Depth();
}


int GSM::MaxDepth( void ) const
{
  return _Stack->MaxDepth();
}



//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

bool GSM::Push( const bool& data )
{
  _Stack->Push( new bool_Portion( data ) );
  return true;
}


bool GSM::Push( const double& data )
{
  _Stack->Push( new numerical_Portion<double>( data ) );
  return true;
}


bool GSM::Push( const gInteger& data )
{
  _Stack->Push( new numerical_Portion<gInteger>( data ) );
  return true;
}


bool GSM::Push( const gRational& data )
{
  _Stack->Push( new numerical_Portion<gRational>( data ) );
  return true;
}


bool GSM::Push( const gString& data )
{
  _Stack->Push( new gString_Portion( data ) );
  return true;
}


/* These are commented out because the don't seem to be necessary */
#if 0
bool GSM::Push( Outcome* data )
{
  _Stack->Push( Outcome_Portion( data ) );
  return true;
}


bool GSM::Push( Player* data )
{
  _Stack->Push( new Player_Portion( data ) );
  return true;
}


bool GSM::Push( Infoset* data )
{
  _Stack->Push( new Infoset_Portion( data ) );
  return true;
}


bool GSM::Push( Action* data )
{
  _Stack->Push( new Action_Portion( data ) );
  return true;
}


bool GSM::Push( Node* data )
{
  _Stack->Push( new Node_Portion( data ) );
  return true;
}
#endif


bool GSM::PushStream( const gString& data )
{
  gOutput* g;
  g = new gFileOutput( data );
  _Stack->Push( new Stream_Portion( *g ) );
  return true;
}


bool GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  List_Portion*  list;
  Portion*       insert_result;
  bool           result = true;

#ifndef NDEBUG
  if( num_of_elements <= 0 )
    _ErrorMessage( _StdErr, 2, num_of_elements );

  if( num_of_elements > _Stack->Depth() )
    _ErrorMessage( _StdErr, 3, num_of_elements, _Stack->Depth() );
#endif // NDEBUG

  list = new List_Portion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _Stack->Pop();
    if( p->Type() == porREFERENCE )
      p = _ResolveRef( (Reference_Portion*) p );

    insert_result = list->Insert( p, 1 );
    if( insert_result != 0 )
    {
      assert( insert_result->Type() == porERROR );
      insert_result->Output( _StdErr );
      delete insert_result;
      result = false;
    }
  }
  _Stack->Push( list );

  return result;
}



//---------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------


bool GSM::PushRef( const gString& ref, const gString& subref )
{
  _Stack->Push( new Reference_Portion( ref, subref ) );
  return true;
}


bool GSM::Assign( void )
{
  Portion*  p2_copy;
  Portion*  p2;
  Portion*  p1;
  Portion*  p;
  Portion*  primary_ref;
  gString   p1_subvalue;
  Portion*  por_result;
  bool      result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
    _ErrorMessage( _StdErr, 4 );
#endif // NDEBUG

  p2 = _Stack->Pop();
  p1 = _Stack->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    p1_subvalue = ( (Reference_Portion*) p1 )->SubValue();

    if( p1_subvalue == "" )
    {
      if( p2->Type() == porREFERENCE )
      {
	p2 = _ResolveRef( (Reference_Portion*) p2 );
	p2_copy = p2->Copy( true );
      }
      else
      {
	p2_copy = p2->Copy();
      }
      _RefTable->Define( ( (Reference_Portion*) p1 )->Value(), p2_copy );
      delete p1;
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

      if( primary_ref->Type() & porALLOWS_SUBVARIABLES )
      {
	if( p2->Type() == porREFERENCE )
	{
	  p2 = _ResolveRef( (Reference_Portion*) p2 );
	  p2_copy = p2->Copy( true );
	}
	else
	{
	  p2_copy = p2->Copy();
	}

	switch( primary_ref->Type() )
	{
	case porNFG_DOUBLE:
	  ( (Nfg_Portion<double>*) primary_ref )->
	    Assign( p1_subvalue, p2_copy );
	  break;
	case porNFG_RATIONAL:
	  ( (Nfg_Portion<gRational>*) primary_ref )->
	    Assign( p1_subvalue, p2_copy );
	  break;
	  
	default:
	  _ErrorMessage( _StdErr, 5 );
	}

	delete p1;
      }
      else // ( !( primary_ref->Type() & porALLOWS_SUBVARIABLES ) )
      {
	_ErrorMessage( _StdErr, 6 );
	if( primary_ref->Type() == porERROR )
	{
	  delete primary_ref;
	}
	delete p2;
	delete p1;
	p2 = new Error_Portion;
      }
    }

    _Stack->Push( p2 );
  }

  else // ( p1->Type() != porREFERENCE )
  {
    int index = 0;
    if( p1->ShadowOf() != 0 )
    {
      index = p1->ShadowOf()->ParentList()->Value().Find( p1->ShadowOf() );
    }
    if( index > 0 )
    {
      por_result = p1->ShadowOf()->ParentList()->SetSubscript( index, p2 );

      if( por_result != 0 )
      {
	por_result->Output( _StdErr );
	delete por_result;
      }

      delete p1;
      _Stack->Push( p2->Copy() );
      result = true;
    }
    else
    {
      _ErrorMessage( _StdErr, 7 );
      delete p1;
      delete p2;
      _Stack->Push( new Error_Portion );
      result = false;
    }
  }
  return result;
}



bool GSM::UnAssign( void )
{
  Portion*  p1;
  Portion*  primary_ref;
  gString   ref;
  gString   p1_subvalue;
  bool      result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 1 )
    _ErrorMessage( _StdErr, 8 );
#endif // NDEBUG

  p1 = _Stack->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    ref = ( (Reference_Portion*) p1 )->Value();
    p1_subvalue = ( (Reference_Portion*) p1 )->SubValue();
    if( p1_subvalue == "" )
    {
      if( _RefTable->IsDefined( ref ) )
      {
	_RefTable->Remove( ref );
      }
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

      if( primary_ref->Type() & porALLOWS_SUBVARIABLES )
      {
	switch( primary_ref->Type() )
	{
	case porNFG_DOUBLE:
	  ( (Nfg_Portion<double>*) primary_ref )->UnAssign( p1_subvalue );
	  break;
	case porNFG_RATIONAL:
	  ( (Nfg_Portion<gRational>*) primary_ref )->UnAssign( p1_subvalue );
	  break;
	  
	default:
	  _ErrorMessage( _StdErr, 9 );
	}
      }
      else
      {
	_ErrorMessage( _StdErr, 10 );
	if( primary_ref->Type() == porERROR )
	{
	  delete primary_ref;
	}
	delete p1;
	p1 = new Error_Portion;
      }
    }
    delete p1;
  }
  else // ( p1->Type() != porREFERENCE )
  {
    _ErrorMessage( _StdErr, 11 );
    result = false;
  }
  return result;
}




//---------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

Portion* GSM::_ResolveRef( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();
  gString&  subvalue = p->SubValue();


  if( _RefTable->IsDefined( ref ) )
  {
    if( subvalue == "" )
    {
      result = (*_RefTable)( ref )->Copy();
    }
    else
    {
      result = (*_RefTable)( ref );
      switch( result->Type() )
      {
      case porNFG_DOUBLE:
	result = ((Nfg_Portion<double>*) result )->operator()( subvalue );
	break;
      case porNFG_RATIONAL:
	result = ((Nfg_Portion<gRational>*) result )->operator()( subvalue );
	break;

      default:
	_ErrorMessage( _StdErr, 12 );
	result = new Error_Portion;
      }
      if( result != 0 )
      {
	if( result->Type() != porERROR )
	  result = result->Copy();
      }
      else
      {
	result = new Error_Portion;
      }
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 13, 0, 0, ref );
    result = new Error_Portion;
  }
  delete p;

  return result;
}


Portion* GSM::_ResolveRefWithoutError( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();
  gString&  subvalue = p->SubValue();

  if( _RefTable->IsDefined( ref ) )
  {
    if( subvalue == "" )
    {
      result = (*_RefTable)( ref )->Copy();
    }
    else
    {
      result = (*_RefTable)( ref );
      switch( result->Type() )
      {
      case porNFG_DOUBLE:
	if( ((Nfg_Portion<double>*) result )->IsDefined( subvalue ) )
	{
	  result = ((Nfg_Portion<double>*) result )->
	    operator()( subvalue )->Copy();
	}
	else
	{
	  result = 0;
	}
	break;
      case porNFG_RATIONAL:
	if( ((Nfg_Portion<gRational>*) result )->IsDefined( subvalue ) )
	{
	  result = ((Nfg_Portion<gRational>*) result )
	    ->operator()( subvalue )->Copy();
	}
	else
	{
	  result = 0;
	}
	break;

      default:
	_ErrorMessage( _StdErr, 14 );
	result = new Error_Portion;
      }
    }
  }
  else
  {
    result = 0;
  }
  delete p;

  return result;
}



Portion* GSM::_ResolvePrimaryRefOnly( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();

  if( _RefTable->IsDefined( ref ) )
  {
    result = (*_RefTable)( ref );
  }
  else
  {
    _ErrorMessage( _StdErr, 15, 0, 0, ref );
    result = new Error_Portion;
  }

  return result;
}



//------------------------------------------------------------------------
//                       binary operations
//------------------------------------------------------------------------


// Main dispatcher of built-in binary operations

// operations are dispatched to the appropriate Portion classes,
// except in cases where the return type is differen from parameter
// #1, in which case the operation implementation would be placed here
// labeled as SPECIAL CASE HANDLING

bool GSM::_BinaryOperation( OperationMode mode )
{
  Portion*   p2;
  Portion*   p1;
  Portion*   p;
  Portion*   result = 0;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
    _ErrorMessage( _StdErr, 16 );
#endif // NDEBUG
  
  p2 = _Stack->Pop();
  p1 = _Stack->Pop();
  
  if( p2->Type() == porREFERENCE )
    p2 = _ResolveRef( (Reference_Portion*) p2 );
  
  if( p1->Type() == porREFERENCE )
    p1 = _ResolveRef( (Reference_Portion*) p1 );


  if( p1->Type() == p2->Type() )
  {
    // SPECIAL CASE HANDLING - Integer division to produce gRationals
    if( mode == opDIVIDE && p1->Type() == porINTEGER &&
       ( (numerical_Portion<gInteger>*) p2 )->Value() != 0 )
    {
      p = new numerical_Portion<gRational>
	( ( (numerical_Portion<gInteger>*) p1 )->Value() );
      ( (numerical_Portion<gRational>*) p )->Value() /=
	( ( (numerical_Portion<gInteger>*) p2 )->Value() );
      delete p2;
      delete p1;
      p1 = p;
    }
    else
    {
      // Main operations dispatcher
      result = p1->Operation( p2, mode );
    }

    // SPECIAL CASE HANDLING - Boolean operators
    if( result != 0 && result->Type() == porBOOL )
    {
      delete p1;
      p1 = result;
      result = 0;
    }
  }

  else // ( p1->Type() != p2->Type() )
  {
    _ErrorMessage( _StdErr, 17, (int) p1->Type(), (int) p2->Type() );
    delete p1;
    delete p2;
    p1 = new Error_Portion;
    result = new Error_Portion;
  }

  _Stack->Push( p1 );

  if( result == 0 )
    return true;
  else
  {
    assert( result->Type() == porERROR );
    if( ( (Error_Portion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    return false;
  }
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation( OperationMode mode )
{
  Portion*  p1;
  Portion*  result = 0;

  if( _Stack->Depth() >= 1 )
  {
    p1 = _Stack->Pop();
    
    if( p1->Type() == porREFERENCE )
      p1 = _ResolveRef( (Reference_Portion*) p1 );

    result = p1->Operation( 0, mode );
    _Stack->Push( p1 );
  }
  else
  {
    _ErrorMessage( _StdErr, 18 );
    result = new Error_Portion;
  }

  if( result == 0 )
    return true;
  else
  {
    assert( result->Type() == porERROR );
    if( ( (Error_Portion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    return false;
  }
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

bool GSM::Add ( void )
{ return _BinaryOperation( opADD ); }

bool GSM::Subtract ( void )
{ return _BinaryOperation( opSUBTRACT ); }

bool GSM::Multiply ( void )
{ return _BinaryOperation( opMULTIPLY ); }

bool GSM::Divide ( void )
{ return _BinaryOperation( opDIVIDE ); }

bool GSM::Negate( void )
{ return _UnaryOperation( opNEGATE ); }


bool GSM::IntegerDivide ( void )
{ return _BinaryOperation( opINTEGER_DIVIDE ); }

bool GSM::Modulus ( void )
{ return _BinaryOperation( opMODULUS ); }


bool GSM::EqualTo ( void )
{ return _BinaryOperation( opEQUAL_TO ); }

bool GSM::NotEqualTo ( void )
{ return _BinaryOperation( opNOT_EQUAL_TO ); }

bool GSM::GreaterThan ( void )
{ return _BinaryOperation( opGREATER_THAN ); }

bool GSM::LessThan ( void )
{ return _BinaryOperation( opLESS_THAN ); }

bool GSM::GreaterThanOrEqualTo ( void )
{ return _BinaryOperation( opGREATER_THAN_OR_EQUAL_TO ); }

bool GSM::LessThanOrEqualTo ( void )
{ return _BinaryOperation( opLESS_THAN_OR_EQUAL_TO ); }


bool GSM::AND ( void )
{ return _BinaryOperation( opLOGICAL_AND ); }

bool GSM::OR ( void )
{ return _BinaryOperation( opLOGICAL_OR ); }

bool GSM::NOT ( void )
{ return _UnaryOperation( opLOGICAL_NOT ); }


bool GSM::Subscript ( void )
{
  Portion* p2;
  Portion* p1;
  Portion* p;
  Portion* refp;
  Portion* real_list;
  Portion* element;
  Portion* shadow;
  bool     result = true;

  assert( _Stack->Depth() >= 2 );
  p2 = _Stack->Pop();
  p1 = _Stack->Pop();

  if( p1->Type() == porREFERENCE )
  {
    refp = p1;
    if( _RefTable->IsDefined( ( (Reference_Portion*) refp )->Value() ) )
      p1 = (*_RefTable)( ( (Reference_Portion*) refp )->Value() );
    else
      p1 = 0;
    
    if( p1 != 0 && p1->Type() == porLIST )
    {
      delete refp;
    }
    else
    {
      p1 = refp;
    }
  }

  if( p2->Type() == porREFERENCE )
    p2 = _ResolveRef( (Reference_Portion*) p2 );

  if( p1->Type() == porLIST )
  {
    if( p2->Type() == porINTEGER )
    {
      if( p1->ShadowOf() == 0 )
      {
	real_list = p1;
      }
      else
      {
	real_list = p1->ShadowOf();
	delete p1;
      }
      element = ( (List_Portion* ) real_list )->
	GetSubscript( ((numerical_Portion<gInteger>*)p2 )->Value().as_long() );
      assert( element != 0 );
      if( element->Type() != porERROR )
      {
	shadow = element->Copy();
	shadow->ShadowOf() = element;
	_Stack->Push( shadow );
      }
      else
      {
	element->Output( _StdErr );
	delete element;
	_Stack->Push( new Error_Portion );
      }
    }
    else
    {
      _ErrorMessage( _StdErr, 19 );
      _Stack->Push( new Error_Portion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 20 );
    delete p1;
    _Stack->Push( new Error_Portion );
    result = false;
  }

  delete p2;
  return result;
}



//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

void GSM::AddFunction( FuncDescObj* func )
{
  _FuncTable->Define( func->FuncName(), func );
}


#ifndef NDEBUG
void GSM::_BindCheck( void ) const
{
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 21 );

  if( _Stack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 22 );
}
#endif // NDEBUG


bool GSM::_BindCheck( const gString& param_name ) const
{
  CallFuncObj*  func;
  int           new_index;
  bool          result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG
  
  func = _CallFuncStack->Peek();
  new_index = func->FindParamName( param_name );
  
  if( new_index >= 0 )
  {
    func->SetCurrParamIndex( new_index );
  }
  else if ( new_index == PARAM_NOT_FOUND )
  {
    _ErrorMessage( _StdErr, 23, 0, 0, param_name, func->FuncName() );
    result = false;
  }
  else // ( new_index == PARAM_AMBIGUOUS )
  {
    _ErrorMessage( _StdErr, 24, 0, 0, param_name, func->FuncName() );
    result = false;
  }
  return result;
}


bool GSM::InitCallFunction( const gString& funcname )
{
  CallFuncObj*  func;
  bool          result = true;

  if( _FuncTable->IsDefined( funcname ) )
  {
    func = new CallFuncObj( (*_FuncTable)( funcname ), _StdErr );
    _CallFuncStack->Push( func );
  }
  else // ( !_FuncTable->IsDefined( funcname ) )
  {
    _ErrorMessage( _StdErr, 25, 0, 0, funcname );
    result = false;
  }
  return result;
}


bool GSM::Bind( void )
{
  return BindRef();
}


bool GSM::BindVal( void )
{
  CallFuncObj*        func;
  PortionType         curr_param_type;
  Portion*            param = 0;
  gString             funcname;
  int                 i;
  int                 type_match;
  gString             ref;
  Reference_Portion*  refp;
  bool                result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Stack->Pop();
  
  if( param->Type() == porREFERENCE )
    param = _ResolveRef( (Reference_Portion *)param );

  param->ShadowOf() = 0;
  result = func->SetCurrParam( param ); 

  if( !result )  // == false
  {
    func->SetErrorOccurred();
  }

  _CallFuncStack->Push( func );
  return result;
}


bool GSM::BindRef( void )
{
  CallFuncObj*  func;
  PortionType   curr_param_type;
  Portion*      param;
  Portion*      subparam;
  gString       funcname;
  int           i;
  int           type_match;
  bool          result = true;
  gString       ref;
  gString       subref;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Stack->Pop();
  
  if( param->Type() == porREFERENCE )
  {
    ref = ( (Reference_Portion*) param )->Value();
    subref = ( (Reference_Portion*) param )->SubValue();
    func->SetCurrParamRef( (Reference_Portion*)( param->Copy() ) );
    param = _ResolveRefWithoutError( (Reference_Portion*) param );
    if( param != 0 )
    {
      if( param->Type() == porERROR )
      {
	delete param;
	result = false;
      }
    }
  }
  else // ( param->Type() != porREFERENCE )
  {
    if( param->ShadowOf() == 0 )
    {
      _CallFuncStack->Push( func );
      _Stack->Push( param );
      result = BindVal();
      return result;
    }
  }

  
  if( result )  // == true
  {
    result = func->SetCurrParam( param );
  }

  if( !result )  // == false
  {
    func->SetErrorOccurred();
  }

  _CallFuncStack->Push( func );
  return result;
}




bool GSM::Bind( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = Bind();
  }
  return result;
}


bool GSM::BindVal( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = BindVal();
  }
  return result;
}


bool GSM::BindRef( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = BindRef();
  }
  return result;
}


bool GSM::CallFunction( void )
{
  CallFuncObj*        func;
  Portion**           param;
  int                 num_params;
  int                 index;
  int                 listindex;
  gString             ref;
  Reference_Portion*  refp;
  Portion*            return_value;
  Portion*            p;
  Portion*            shadowof;
  Portion*            por_result;
  bool                result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 26 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  num_params = func->NumParams();
  param = new Portion*[ num_params ];

  return_value = func->CallFunction( param );

  if( return_value == 0 )
  {
    _ErrorMessage( _StdErr, 27, 0, 0, func->FuncName() );
    return_value = new Error_Portion;
    result = false;
  }

  _Stack->Push( return_value );


  for( index = 0; index < num_params; index++ )
  {
    if( func->ParamPassByReference( index ) )
    {
      func->SetCurrParamIndex( index );
      refp = func->GetCurrParamRef();

      if( refp != 0 && param[ index ] != 0 )
      {
	if( refp->SubValue() == "" )
	{
	  _RefTable->Define( refp->Value(), param[ index ] );
	}
	else // ( refp->SubValue != "" )
	{
	  if( _RefTable->IsDefined( refp->Value() ) )
	  {
	    p = ( *_RefTable )( refp->Value() );
	    switch( p->Type() )
	    {
	    case porNFG_DOUBLE:
	      ( (Nfg_Portion<double>*) p )->
		Assign( refp->SubValue(), param[ index ]->Copy() );
	      break;
	    case porNFG_RATIONAL:
	      ( (Nfg_Portion<gRational>*) p )->
		Assign( refp->SubValue(), param[ index ]->Copy() );
	      break;

	    default:
	      _ErrorMessage( _StdErr, 28 );
	      result = false;
	    }
	    delete param[ index ];
	  }
	  else // ( !_RefTable->IsDefined( refp->Value() ) )
	  {
	    _ErrorMessage( _StdErr, 29 );
	    delete param[ index ];
	    result = false;
	  }
	}
	delete refp;
      }
      else // ( !( refp != 0 && param[ index ] != 0 ) )
      {
	if( ( refp == 0 ) && ( param[ index ] != 0 ) )
	{
	  listindex = 0;
	  shadowof = func->GetCurrParamShadowOf();
	  if( shadowof != 0 )
	  {
	    listindex = shadowof->ParentList()->Value().Find( shadowof );
	    if( listindex > 0 )
	    {
	      por_result = shadowof->ParentList()->
		SetSubscript( listindex, param[index]->Copy() );

	      if( por_result != 0 )
	      {
		por_result->Output( _StdErr );
		delete por_result;
	      }
	    }
#ifndef NDEBUG
	    else
	    {
	      _ErrorMessage( _StdErr, 30 );
	    }
#endif // NDEBUG
	  }
	  delete param[ index ];
	}
#ifndef NDEBUG
	else if( ( refp != 0 ) && ( param[ index ] == 0 ) )
	{
	  _ErrorMessage( _StdErr, 31, index, 0, func->FuncName(), "", refp );
	}
#endif // NDEBUG
      }
    }
  }

  delete func;

  delete[] param;
  
  return result;
}


//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

GSM_ReturnCode GSM::Execute( gList< Instruction* >& program )
{
  GSM_ReturnCode  result          = rcSUCCESS;
  bool            instr_success;
  bool            done            = false;
  Portion*        p;
  Instruction*    instruction;
  int             program_counter = 1;
  int             program_length  = program.Length();

  while( ( program_counter <= program_length ) && ( !done ) )
  {
    instruction = program[ program_counter ];

    switch( instruction->Type() )
    {
    case iQUIT:
      instr_success = true;
      result = rcQUIT;
      done = true;
      break;

    case iIF_GOTO:
      p = _Stack->Pop();
      if( p->Type() == porBOOL )
      {
	if( ( (bool_Portion*) p )->Value() )
	{
	  program_counter = ( (IfGoto*) instruction )->WhereTo();
	  assert( program_counter >= 1 && program_counter <= program_length );
	}
	else
	{
	  program_counter++;
	}
	delete p;
	instr_success = true;
      }
      else
      {
	_ErrorMessage( _StdErr, 32 );
	_Stack->Push( p );
	program_counter++;
	instr_success = false;
      }
      break;

    case iGOTO:
      program_counter = ( (Goto*) instruction )->WhereTo();
      assert( program_counter >= 1 && program_counter <= program_length );
      instr_success = true;
      break;

    default:
      instr_success = instruction->Execute( *this );
      program_counter++;
    }

    if( !instr_success )
    {
      _ErrorMessage( _StdErr, 33, program_counter, 0, "", "", 0, instruction );
      result = rcFAIL;
      break;
    }
  }

  while( program.Length() > 0 )
  {
    instruction = program.Remove( 1 );
    delete instruction;
  }

  return result;
}


//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------


bool GSM::Pop( void )
{
  Portion* p;
  bool result = false;

  if( _Stack->Depth() > 0 )
  {
    p = _Stack->Pop();
    delete p;
    result = true;
  }
  else
  {
    _ErrorMessage( _StdErr, 34 );
  }
  return result;
}


void GSM::Output( void )
{
  Portion*  p;

  assert( _Stack->Depth() >= 0 );

  if( _Stack->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    p = _Stack->Pop();
    if( p->Type() == porREFERENCE )
      p = _ResolveRef( (Reference_Portion*) p );
    p->Output( _StdOut );
    _StdOut << "\n";
    delete p;
  }
}


void GSM::Dump( void )
{
  int  i;

  assert( _Stack->Depth() >= 0 );

  if( _Stack->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    for( i = _Stack->Depth() - 1; i >= 0; i-- )
    {
      _StdOut << "Stack element " << i << " : ";
      Output();
    }
  }
  _StdOut << "\n";
  
  assert( _Stack->Depth() == 0 );
}


void GSM::Flush( void )
{
  int       i;
  Portion*  p;

  for( i = _Stack->Depth() - 1; i >= 0; i-- )
  {
    p = _Stack->Pop();
    delete p;
  }

  assert( _Stack->Depth() == 0 );
}



//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 gOutput&        s,
 const int       error_num,
 const gInteger& num1, 
 const gInteger& num2,
 const gString&  str1,
 const gString&  str2,
 Portion*        por,
 Instruction*    instr
 )
{
  s << "GSM Error " << error_num << " :\n";

  switch( error_num )
  {
  case 1:
    s << "  Illegal stack size specified during initialization\n";
    s << "  Stack size requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 2:
    s << "  Illegal number of elements requested to PushList()\n";
    s << "  Elements requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 3:
    s << "  Not enough elements in GSM to PushList()\n";
    s << "  Elements requested: " << num1 << "\n";
    s << "  Elements available: " << num2 << "\n";
    assert( 0 );
    break;
  case 4:
    s << "  Not enough operands to execute Assign()\n";
    assert( 0 );
    break;
  case 5:
    s << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 6:
    s << "  Attempted to assign a sub-reference to a type\n";
    s << "  that doesn't support such structures\n";
    break;
  case 7:
    s << "  No reference found to be assigned\n";
    break;
  case 8:
    s << "  Not enough operands to execute UnAssign()\n";
    assert( 0 );
    break;
  case 9:
    s << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 10:
    s << "  Attempted to unassign a sub-reference of a type\n";
    s << "  that doesn't support such structures\n";
    break;
  case 11:
    s << "  No reference found to be unassigned\n";
    break;
  case 12:
    s << "  Attempted to resolve a subvariable of a type\n";
    s << "  that does not support subvariables\n";
    break;
  case 13:
    s << "  Attempted to resolve an undefined reference";
    s << " \"" << str1 << "\"\n";
    break;
  case 14:
    s << "  Attempted to resolve the subvariable of a type\n";
    s << "  that does not support subvariables\n";
    break;
  case 15:
    s << "  Attempted to resolve an undefined reference\n";
    s << "  \"" << str1 << "\"\n";
    break;
  case 16:
    s << "  Not enough operands to perform binary operation\n";
    assert( 0 );
    break;
  case 17:
    s << "  Attempted operating on different types\n";
    s << "  Type of Operand 1: ";
    PrintPortionTypeSpec( s, (PortionType) num1.as_long() );
    s << "  Type of Operand 2: ";
    PrintPortionTypeSpec( s, (PortionType) num2.as_long() );
    break;
  case 18:
    s << "  Not enough operands to perform unary operation\n";
    break;
  case 19:
    s << "  A non-integer element number passed as the\n";
    s << "  subscript of a List\n";
    break;
  case 20:
    s << "  Attempted to take the subscript of a non-List\n";
    s << "  Portion type\n";
    break;
  case 21:
    s << "  The CallFunction() subsystem was not initialized by\n";
    s << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 22:
    s << "  No value found to assign to a function parameter\n";
    assert( 0 );
    break;
  case 23:
    s << "  Parameter \"" << str1 << "\" is not defined for\n";
    s << "  the function \"" << str2 << "\"\n";
    break;
  case 24:
    s << "  Parameter \"" << str1 << "\" is ambiguous in\n";
    s << "  the function \"" << str2 << "\"\n";
    break;
  case 25:
    s << "  Undefined function name:\n";
    s << "  InitCallFunction( \"" << str1 << "\" )\n";
    break;
  case 26:
    s << "  The CallFunction() subsystem was not initialized by\n";
    s << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 27:
    s << "  An error occurred while attempting to execute\n";
    s << "  CallFunction( \"" << str1 << "\", ... )\n";
    break;
  case 28:
    s << "  Attempted to assign the subvariable of a\n";
    s << "  type that does not support subvariables\n";
    break;
  case 29:
    s << "  Attempted to assign the sub-variable of\n";
    s << "  an undefined variable\n";
    break;
  case 30:
    s << "  Fatal Error:\n";
    s << "    Returning function parameter information\n";
    s << "    (regarding lists) is invalid\n";
    assert(0);
    break;
  case 31:
    s << "  Fatal Error:\n";
    s << "    Attempting to assign a null Portion to a reference\n";
    s << "    Function: \"" << str1 << "\"\n";
    s << "    Parameter index: " << num1 << "\n";
    s << "    Reference: " << por << "\n";
    assert(0);
    break;
  case 32:
    s << "  Instruction IfGoto called on a non-boolean data type\n";
    break;
  case 33:
    s << "  Instruction #" << num1 << ": " << instr << "\n";
    s << "  was not executed successfully\n";
    s << "  Program abnormally terminated.\n";
    break;
  case 34:
    s << "  Pop() called on an empty stack\n";
    break;
  default:
    s << "  General error\n";
  }
}






//-----------------------------------------------------------------------
//                       Template instantiations
//-----------------------------------------------------------------------



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__




#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< CallFuncObj* >;
TEMPLATE class gStack< List_Portion* >;


#include "ggrstack.imp"

TEMPLATE class gGrowableStack< Portion* >;
TEMPLATE class gGrowableStack< CallFuncObj* >;
TEMPLATE class gGrowableStack< List_Portion* >;


gOutput& operator << ( class gOutput& s, class Portion* (*funcname)() )
{ return s << funcname; }




