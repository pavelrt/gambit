//
// FILE: solfunc.cc -- GCL functions on profiles and solutions
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"
#include "nfg.h"
#include "behavsol.h"
#include "mixedsol.h"


//
// Implementations of these are provided as necessary in gsmutils.cc
//
template <class T> Portion *ArrayToList(const gArray<T> &);
// these added to get around g++ 2.7.2 not properly completing type
// unification when gVector passed to the parameter...
extern Portion *ArrayToList(const gArray<double> &);
extern Portion *ArrayToList(const gArray<gRational> &);
template <class T> Portion *ArrayToList(const gList<T> &);
template <class T> Portion *gDPVectorToList(const gDPVector<T> &);

//------------------
// ActionValues
//------------------

Portion *GSM_ActionValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<double> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_ActionValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<gRational> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

//--------------
// Behav
//--------------

Portion *GSM_Behav_EfgFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().ListDepth == 0 )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Spec().Type != porFLOAT )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;

}

Portion *GSM_Behav_EfgRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().ListDepth == 0 )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Spec().Type != porRATIONAL )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_Behav_EFSupport(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por = 0;


  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  // This is incredibly redundent; must find a way to reuse the code
  // from the previous two functions.
  if (S->BelongsTo().Type() == DOUBLE && param[1]->Spec().Type & porFLOAT )
  {
    // The code here is completely copied from GSM_Behav_EfgFloat

    Efg<double> &E = * (Efg<double>*) &S->BelongsTo();
    BehavSolution<double> *P = new BehavSolution<double>(E);

    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Spec().ListDepth == 0 )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Spec().ListDepth == 0 )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Spec().Type != porFLOAT )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }
    por = new BehavValPortion(P);


  }
  else if (S->BelongsTo().Type()== RATIONAL && param[1]->Spec().Type & porRATIONAL )
  {
    // The code here is entirely copied from GSM_Behav_EfgRational()

    Efg<gRational> &E = * (Efg<gRational>*) &S->BelongsTo();
    BehavSolution<gRational> *P = new BehavSolution<gRational>(E);
    
    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Spec().ListDepth == 0 )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Spec().ListDepth == 0 )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Spec().Type != porRATIONAL )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }

    por = new BehavValPortion(P);
  }

  if( por == 0 )
    por = new ErrorPortion( "Mismatching EFG and list type" );
  else
  {
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
  }
  return por;
}

//-------------
// Beliefs
//-------------

Portion *GSM_BeliefsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

//--------------
// Centroid
//--------------
 
Portion *GSM_CentroidEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEFSupport(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  BaseBehavProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new BehavSolution<double>( *S );
  else
    P = new BehavSolution<gRational>( *S );

  Portion *por = new BehavValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<double> *P = new MixedSolution<double>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<gRational> *P = new MixedSolution<gRational>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNFSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  BaseMixedProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S);
  else
    P = new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);

  Portion *por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//---------------
// GobitLambda
//---------------

Portion* GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

//--------------
// GobitValue
//--------------

Portion* GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}

//----------------
// InfosetProbs
//----------------

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_InfosetProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}


//----------------
// IsNash
//----------------

Portion *GSM_IsNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}


//----------------
// IsntNash
//----------------

Portion *GSM_IsntNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

//----------------
// IsntPerfect
//----------------

Portion *GSM_IsntPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

Portion *GSM_IsntPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

//--------------
// IsntProper
//--------------

Portion *GSM_IsntProper_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_NO);
}

Portion *GSM_IsntProper_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_NO);
}

//-------------
// IsPerfect
//-------------

Portion *GSM_IsPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

Portion *GSM_IsPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

//------------
// IsProper
//------------

Portion *GSM_IsProper_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_YES);
}

Portion *GSM_IsProper_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_YES);
}

//----------------
// IsSequential
//----------------

Portion *GSM_IsSequential_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

Portion *GSM_IsSequential_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

//--------------------
// IsSubgamePerfect
//--------------------

Portion *GSM_IsSubgamePerfect_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

Portion *GSM_IsSubgamePerfect_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

//----------------
// LiapValue
//----------------

Portion *GSM_LiapValue_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new FloatValPortion(P->LiapValue());
}

Portion *GSM_LiapValue_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new RationalValPortion(P->LiapValue());
}


Portion *GSM_LiapValue_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new FloatValPortion(P->LiapValue());
}

Portion *GSM_LiapValue_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new RationalValPortion(P->LiapValue());
}

//---------------
// ListForm
//---------------

Portion *GSM_ListForm_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return gDPVectorToList(* (gDPVector<double>*) P);
}


Portion *GSM_ListForm_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return gDPVectorToList(* (gDPVector<gRational>*) P);
}

Portion *GSM_ListForm_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new FloatValPortion( (*P)( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}


Portion *GSM_ListForm_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new RationalValPortion( (*P)( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}



//---------------------------- Gripe ------------------------------//

Portion *GSM_Gripe_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();

  gPVector<double> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for(i=1; i <= P->Lengths().Length(); i++)
  {
    p1 = new ListValPortion();

    for(j=1; j <= P->Lengths()[i]; j++)
    {
      p2 = new FloatValPortion(v(i, j));
      ((ListValPortion*) p1)->Append(p2);
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}


Portion *GSM_Gripe_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();

  gPVector<gRational> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new RationalValPortion( v( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}

Portion *GSM_Gripe_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();

  gDPVector<double> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new FloatValPortion( v( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

}


Portion *GSM_Gripe_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();

  gDPVector<gRational> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new RationalValPortion( v( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

}


//----------
// Mixed
//----------

Portion *GSM_Mixed_NfgFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<double> *P = new MixedSolution<double>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != porFLOAT )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (FloatPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion *GSM_Mixed_NfgRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<gRational> *P = new MixedSolution<gRational>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != porRATIONAL )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion* GSM_Mixed_NFSupport( Portion** param )
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gArray<int> dim = S->SupportDimensions();
  BaseMixedProfile *P;
  Portion* por;
  unsigned long datatype;
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  switch( param[ 0 ]->Owner()->Spec().Type )
  {
  case porNFG_FLOAT:
    P = new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S);
    datatype = porFLOAT;
    break;
  case porNFG_RATIONAL:
    P = new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);
    datatype = porRATIONAL;
    break;
  default:
    assert( 0 );
  }


  if( ( (ListPortion*) param[1] )->Length() != dim.Length() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= dim.Length(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != dim[ i ] )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }
    
    for( j = 1; j <= dim[ i ]; j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != datatype )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      switch( datatype )
      {
      case porFLOAT:
	( * (MixedSolution<double>*) P )( i, j ) = 
	  ( (FloatPortion*) p2 )->Value();
	break;
      case porRATIONAL:
	( * (MixedSolution<gRational>*) P )( i, j ) = 
	  ( (RationalPortion*) p2 )->Value();
	break;
      default:
	assert( 0 );
      }
      
      delete p2;
    }
    delete p1;
  }
  
  por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//----------------
// NodeValues
//----------------

Portion *GSM_NodeValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 
//----------------
// RealizProbs
//----------------

Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}

//-----------------
// SetComponent
//-----------------

Portion *GSM_SetComponent_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<double>* P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<double>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Spec().ListDepth > 0 )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Spec().Type == porFLOAT );
    (*P)( PlayerNum, InfosetNum, k ) = ( (FloatPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<gRational>* P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<gRational>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Spec().ListDepth > 0 )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Spec().Type == porRATIONAL );
    (*P)( PlayerNum, InfosetNum, k ) = ( (RationalPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}

Portion *GSM_SetComponent_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<double>* P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<double>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Spec().ListDepth > 0 )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Spec().Type == porFLOAT );
    (*P)( PlayerNum, j ) = ( (FloatPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<gRational>* P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<gRational>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Spec().ListDepth > 0 )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Spec().Type == porRATIONAL );
    (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}

//---------------
// Support
//---------------

Portion* GSM_Support_Behav(Portion** param)
{
  BaseBehavProfile *P = ((BehavPortion*) param[0])->Value();
  return new EfSupportValPortion(new EFSupport(P->GetEFSupport()));
}

Portion* GSM_Support_Mixed(Portion** param)
{
  BaseMixedProfile *P = ((MixedPortion *) param[0])->Value();
  return new NfSupportValPortion(new NFSupport(P->GetNFSupport()));
}




void Init_solfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ActionValues", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValuesFloat, 
				       PortionSpec(porFLOAT, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionValuesRational, 
				       PortionSpec(porRATIONAL, 1), 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Behav", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Behav_EfgFloat, 
				       porBEHAV_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("list", PortionSpec(porFLOAT, 3),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Behav_EfgRational, 
				       porBEHAV_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("list", 
					    PortionSpec(porRATIONAL, 3),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Behav_EFSupport, 
				       porBEHAV, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porEF_SUPPORT,
					    REQUIRED));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Beliefs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_BeliefsFloat, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_BeliefsRational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Centroid", 6);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CentroidEfgFloat, 
				       porBEHAV_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_CentroidEfgRational, 
				       porBEHAV_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_CentroidEFSupport, 
				       porBEHAV, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porEF_SUPPORT));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_CentroidNfgFloat, 
				       porMIXED_FLOAT, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("nfg", porNFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_CentroidNfgRational,
				       porMIXED_RATIONAL, 1));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("nfg", porNFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_CentroidNFSupport,
				       porMIXED, 1));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("support", porNF_SUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitLambda", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitLambda_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitLambda_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porMIXED_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GobitLambda_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GobitLambda_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("GobitValue", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitValue_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porMIXED_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GobitValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GobitValue_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("InfosetProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProbsFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InfosetProbsRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsntNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsntNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsntNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  
  FuncObj = new FuncDescObj("IsntPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsntProper", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntProper_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntProper_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porMIXED_RATIONAL,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsProper", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsProper_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsProper_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED_RATIONAL,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSequential", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSequential_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsSequential_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSubgamePerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSubgamePerfect_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsSubgamePerfect_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapValue", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LiapValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));  
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LiapValue_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LiapValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));  
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_LiapValue_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy",	porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


   
  FuncObj = new FuncDescObj("ListForm", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ListForm_BehavFloat, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ListForm_BehavRational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_ListForm_MixedFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_ListForm_MixedRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Gripe", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Gripe_BehavFloat, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Gripe_BehavRational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Gripe_MixedFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Gripe_MixedRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Mixed", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Mixed_NfgFloat, 
				       porMIXED_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT, 2),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Mixed_NfgRational, 
				       porMIXED_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("list", 
					    PortionSpec(porRATIONAL, 2),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Mixed_NFSupport, 
				       porMIXED, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 1)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NodeValues", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValuesFloat, 
				       PortionSpec(porFLOAT, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porPLAYER_EFG));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NodeValuesRational, 
				       PortionSpec(porRATIONAL, 1), 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porPLAYER_EFG));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("RealizProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProbsFloat, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RealizProbsRational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("SetComponent", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetComponent_BehavFloat, 
				       porBEHAV_FLOAT, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("list", PortionSpec(porFLOAT, 1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetComponent_BehavRational, 
				       porBEHAV_RATIONAL, 3));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("list", 
					    PortionSpec(porRATIONAL,1)));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_SetComponent_MixedFloat, 
				       porMIXED_FLOAT, 3));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(2, 2, ParamInfoType("list", PortionSpec(porFLOAT,1)));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_SetComponent_MixedRational, 
				       porMIXED_RATIONAL, 3));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("list", 
					    PortionSpec(porRATIONAL,1)));

  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("Support", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Behav, 
				       porEF_SUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Support_Mixed, 
				       porNF_SUPPORT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);
}

