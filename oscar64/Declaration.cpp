#include "Declaration.h"

DeclarationScope::DeclarationScope(DeclarationScope* parent, ScopeLevel level, const Ident* name)
{
	mParent = parent;
	mLevel = level;
	mName = name;
	mHashSize = 0;
	mHashFill = 0;
	mHash = nullptr;
}

DeclarationScope::~DeclarationScope(void)
{
	delete[] mHash;
}

const Ident* DeclarationScope::Mangle(const Ident* ident) const
{
	if (mName && ident)
	{
		char	buffer[200];
		strcpy_s(buffer, mName->mString);
		strcat_s(buffer, "::");
		strcat_s(buffer, ident->mString);
		return Ident::Unique(buffer);
	}
	else
		return ident;
}

void DeclarationScope::UseScope(DeclarationScope* scope)
{
	mUsed.Push(scope);
}

Declaration * DeclarationScope::Insert(const Ident* ident, Declaration* dec)
{
	if (!mHash)
	{
		mHashSize = 16;
		mHashFill = 0;
		mHash = new Entry[mHashSize];
		for (int i = 0; i < mHashSize; i++)
		{
			mHash[i].mDec = nullptr;
			mHash[i].mIdent = nullptr;
		}
	}

	int		hm = mHashSize - 1;
	int		hi = ident->mHash & hm;

	while (mHash[hi].mIdent)
	{
		if (ident == mHash[hi].mIdent)
			return mHash[hi].mDec;
		hi = (hi + 1) & hm;
	}

	mHash[hi].mIdent = ident;
	mHash[hi].mDec = dec;
	mHashFill++;

	if (2 * mHashFill >= mHashSize)
	{
		int		size  = mHashSize;
		Entry* entries = mHash;
		mHashSize *= 2;
		mHashFill = 0;
		mHash = new Entry[mHashSize];
		for (int i = 0; i < mHashSize; i++)
		{
			mHash[i].mDec = nullptr;
			mHash[i].mIdent = nullptr;
		}
		for (int i = 0; i < size; i++)
		{
			if (entries[i].mIdent)
				Insert(entries[i].mIdent, entries[i].mDec);
		}
		delete[] entries;
	}

	return nullptr;
}

Declaration* DeclarationScope::Lookup(const Ident* ident, ScopeLevel limit)
{
	if (mLevel < limit)
		return nullptr;

	if (mHashSize > 0)
	{
		unsigned int		hm = mHashSize - 1;
		unsigned int		hi = ident->mHash & hm;

		while (mHash[hi].mIdent)
		{
			if (ident == mHash[hi].mIdent)
				return mHash[hi].mDec;
			hi = (hi + 1) & hm;
		}
	}

	if (limit == SLEVEL_SCOPE)
		return nullptr;

	for (int i = 0; i < mUsed.Size(); i++)
	{
		Declaration* dec = mUsed[i]->Lookup(ident, limit);
		if (dec)
			return dec;
	}

	return mParent ? mParent->Lookup(ident, limit) : nullptr;
}

void DeclarationScope::End(const Location& loc)
{
	for (int i = 0; i < mHashSize; i++)
	{
		if (mHash[i].mDec)
			mHash[i].mDec->mEndLocation = loc;
	}
}

Expression::Expression(const Location& loc, ExpressionType type)
	:	mLocation(loc), mEndLocation(loc), mType(type), mLeft(nullptr), mRight(nullptr), mConst(false), mDecType(nullptr), mDecValue(nullptr)
{

}

Expression::~Expression(void)
{

}

void Expression::Dump(int ident) const
{
	for (int i = 0; i < ident; i++)
		printf("|  ");

	switch (mType)
	{
	case EX_ERROR:
		printf("ERROR");
		break;
	case EX_VOID:
		printf("VOID");
		break;
	case EX_CONSTANT:
		printf("CONST");
		if (mDecValue->mIdent)
			printf(" '%s'", mDecValue->mIdent->mString);
		break;
	case EX_VARIABLE:
		printf("VAR");
		if (mDecValue->mIdent)
			printf(" '%s'", mDecValue->mIdent->mString);
		break;
	case EX_ASSIGNMENT:
		printf("ASSIGN<%s>", TokenNames[mToken]);
		break;
	case EX_INITIALIZATION:
		printf("INIT");
		break;
	case EX_BINARY:
		printf("BINARY<%s>", TokenNames[mToken]);
		break;
	case EX_RELATIONAL:
		printf("RELATIONAL<%s>", TokenNames[mToken]);
		break;
	case EX_PREINCDEC:
		printf("PREOP<%s>", TokenNames[mToken]);
		break;
	case EX_PREFIX:
		printf("PREFIX<%s>", TokenNames[mToken]);
		break;
	case EX_POSTFIX:
		printf("POSTFIX<%s>", TokenNames[mToken]);
		break;
	case EX_POSTINCDEC:
		printf("POSTOP<%s>", TokenNames[mToken]);
		break;
	case EX_INDEX:
		printf("INDEX");
		break;
	case EX_QUALIFY:
		printf("QUALIFY");
		break;
	case EX_CALL:
		printf("CALL");
		break;
	case EX_INLINE:
		printf("INLINE");
		break;
	case EX_VCALL:
		printf("VCALL");
		break;
	case EX_DISPATCH:
		printf("DISPATCH");
		break;
	case EX_LIST:
		printf("LIST");
		break;
	case EX_RETURN:
		printf("RETURN");
		break;
	case EX_SEQUENCE:
		printf("SEQUENCE");
		break;
	case EX_WHILE:
		printf("WHILE");
		break;
	case EX_IF:
		printf("IF");
		break;
	case EX_ELSE:
		printf("ELSE");
		break;
	case EX_FOR:
		printf("FOR");
		break;
	case EX_DO:
		printf("DO");
		break;
	case EX_SCOPE:
		printf("SCOPE");
		break;
	case EX_BREAK:
		printf("BREAK");
		break;
	case EX_CONTINUE:
		printf("CONTINUE");
		break;
	case EX_TYPE:
		printf("TYPE");
		break;
	case EX_TYPECAST:
		printf("TYPECAST");
		break;
	case EX_LOGICAL_AND:
		printf("AND");
		break;
	case EX_LOGICAL_OR:
		printf("OR");
		break;
	case EX_LOGICAL_NOT:
		printf("NOT");
		break;
	case EX_ASSEMBLER:
		printf("ASSEMBLER");
		break;
	case EX_UNDEFINED:
		printf("UNDEFINED");
		break;
	case EX_SWITCH:
		printf("SWITCH");
		break;
	case EX_CASE:
		printf("CASE");
		break;
	case EX_DEFAULT:
		printf("DEFAULT");
		break;
	case EX_CONDITIONAL:
		printf("COND");
		break;
	case EX_ASSUME:
		printf("ASSUME");
		break;
	case EX_BANKOF:
		printf("BANKOF");
		break;
	case EX_CONSTRUCT:
		printf("CONSTRUCT");
		break;
	case EX_CLEANUP:
		printf("CLEANUP");
		break;
	case EX_RESULT:
		printf("RESULT");
		break;
	}
	printf("\n");

	if (mLeft)
		mLeft->Dump(ident + 1);
	if (mRight)
		mRight->Dump(ident + 1);
}

bool Expression::HasSideEffects(void) const
{
	switch (mType)
	{
	case EX_VARIABLE:
	case EX_CONSTANT:
		return false;

	case EX_BINARY:
	case EX_RELATIONAL:
	case EX_INDEX:
		return mLeft->HasSideEffects() || mRight->HasSideEffects();

	case EX_QUALIFY:
	case EX_TYPECAST:
	case EX_PREFIX:
	case EX_POSTFIX:
		return mLeft->HasSideEffects();

	default:
		return true;
	}
}

bool Expression::IsSame(const Expression* exp) const
{
	if (!exp || mType != exp->mType)
		return false;

	switch (mType)
	{
	case EX_VARIABLE:
		return mDecValue == exp->mDecValue;

	default:
		return false;
	}
}

Expression* Expression::LogicInvertExpression(void) 
{
	if (mType == EX_LOGICAL_NOT)
		return mLeft;
	else if (mType == EX_LOGICAL_AND)
	{
		mType = EX_LOGICAL_OR;
		mLeft = mLeft->LogicInvertExpression();
		mRight = mRight->LogicInvertExpression();
		return this;
	}
	else if (mType == EX_LOGICAL_OR)
	{
		mType = EX_LOGICAL_AND;
		mLeft = mLeft->LogicInvertExpression();
		mRight = mRight->LogicInvertExpression();
		return this;
	}
	else if (mType == EX_RELATIONAL)
	{
		switch (mToken)
		{
		case TK_EQUAL:
			mToken = TK_NOT_EQUAL;
			break;
		case TK_NOT_EQUAL:
			mToken = TK_EQUAL;
			break;
		case TK_GREATER_THAN:
			mToken = TK_LESS_EQUAL;
			break;
		case TK_GREATER_EQUAL:
			mToken = TK_LESS_THAN;
			break;
		case TK_LESS_THAN:
			mToken = TK_GREATER_EQUAL;
			break;
		case TK_LESS_EQUAL:
			mToken = TK_GREATER_THAN;
			break;
		}

		return this;
	}
	else
	{
		Expression* ex = new Expression(mLocation, EX_LOGICAL_NOT);
		ex->mLeft = this;
		ex->mDecType = TheBoolTypeDeclaration;
		return ex;
	}
}

Expression* Expression::ConstantFold(Errors * errors)
{
	if (mType == EX_PREFIX && mLeft->mType == EX_CONSTANT)
	{
		if (mLeft->mDecValue->mType == DT_CONST_INTEGER)
		{
			switch (mToken)
			{
			case TK_ADD:
				return mLeft;
			case TK_SUB:
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration	*	dec = new Declaration(mLocation, DT_CONST_INTEGER);
				if (mLeft->mDecValue->mBase->mSize <= 2)
					dec->mBase = TheSignedIntTypeDeclaration;
				else
					dec->mBase = TheSignedLongTypeDeclaration;
				dec->mInteger = - mLeft->mDecValue->mInteger;
				ex->mDecValue = dec;
				ex->mDecType = dec->mBase;
				return ex;
			}
			case TK_BINARY_NOT:
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_INTEGER);
				dec->mBase = mLeft->mDecValue->mBase;
				dec->mInteger = ~mLeft->mDecValue->mInteger;
				ex->mDecValue = dec;
				ex->mDecType = dec->mBase;
				return ex;
			}

			}
		}
		else if (mLeft->mDecValue->mType == DT_CONST_FLOAT)
		{
			switch (mToken)
			{
			case TK_ADD:
				return mLeft;
			case TK_SUB:
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_FLOAT);
				dec->mBase = TheFloatTypeDeclaration;
				dec->mNumber = -mLeft->mDecValue->mNumber;
				ex->mDecValue = dec;
				ex->mDecType = TheFloatTypeDeclaration;
				return ex;
			}

			}
		}
		else if (mLeft->mDecValue->mType == DT_CONST_FUNCTION)
		{
			switch (mToken)
			{
			case TK_BINARY_AND:
				return mLeft;
			}
		}
	}
#if 1
	else if (mType == EX_PREFIX && mToken == TK_BINARY_AND && mLeft->mType == EX_VARIABLE && (mLeft->mDecValue->mFlags & (DTF_STATIC | DTF_GLOBAL)))
	{
		Expression* ex = new Expression(mLocation, EX_CONSTANT);
		Declaration* dec = new Declaration(mLocation, DT_CONST_POINTER);
		dec->mValue = mLeft;
		dec->mBase = mDecType;
		ex->mDecValue = dec;
		ex->mDecType = mDecType;
		return ex;
	}
#endif
	else if (mType == EX_TYPECAST && mLeft->mType == EX_CONSTANT)
	{
		if (mDecType->mType == DT_TYPE_POINTER)
		{
			if (mLeft->mDecValue->mType == DT_CONST_ADDRESS || mLeft->mDecValue->mType == DT_CONST_INTEGER)
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_ADDRESS);
				dec->mBase = mDecType;
				dec->mInteger = mLeft->mDecValue->mInteger;
				ex->mDecValue = dec;
				ex->mDecType = mDecType;
				return ex;
			}
			else if (mLeft->mDecValue->mType == DT_CONST_FUNCTION)
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				ex->mDecValue = mLeft->mDecValue;
				ex->mDecType = mDecType;
				return ex;
			}
		}
		else if (mDecType->mType == DT_TYPE_INTEGER)
		{
			if (mLeft->mDecValue->mType == DT_CONST_FLOAT)
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_INTEGER);
				dec->mBase = mDecType;
				dec->mInteger = int64(mLeft->mDecValue->mNumber);
				ex->mDecValue = dec;
				ex->mDecType = mDecType;
				return ex;
			}
			else if (mLeft->mDecValue->mType == DT_CONST_INTEGER)
			{
				int64	sval = 1ULL << (8 * mDecType->mSize);
				int64	v = mLeft->mDecValue->mInteger & (sval - 1);

				if (mDecType->mFlags & DTF_SIGNED)
				{
					if (v & (sval >> 1))
						v -= sval;
				}

				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_INTEGER);
				dec->mBase = mDecType;
				dec->mInteger = v;
				ex->mDecValue = dec;
				ex->mDecType = mDecType;
				return ex;
			}
		}
		else if (mDecType->mType == DT_TYPE_FLOAT)
		{
			if (mLeft->mDecValue->mType == DT_CONST_INTEGER)
			{
				Expression* ex = new Expression(mLocation, EX_CONSTANT);
				Declaration* dec = new Declaration(mLocation, DT_CONST_FLOAT);
				dec->mBase = mDecType;
				dec->mNumber = double(mLeft->mDecValue->mInteger);
				ex->mDecValue = dec;
				ex->mDecType = mDecType;
				return ex;
			}
		}
	}
	else if (mType == EX_BINARY && mLeft->mType == EX_CONSTANT && mRight->mType == EX_CONSTANT)
	{
		if (mLeft->mDecValue->mType == DT_CONST_INTEGER && mRight->mDecValue->mType == DT_CONST_INTEGER)
		{
			int64	ival = 0, ileft = mLeft->mDecValue->mInteger, iright = mRight->mDecValue->mInteger;

			switch (mToken)
			{
			case TK_ADD:
				ival = ileft + iright;
				break;
			case TK_SUB:
				ival = ileft - iright;
				break;
			case TK_MUL:
				ival = ileft * iright;
				break;
			case TK_DIV:
				if (iright == 0)
					errors->Error(mLocation, EERR_INVALID_VALUE, "Constant division by zero");
				else
					ival = ileft / iright;
				break;
			case TK_MOD:
				if (iright == 0)
					errors->Error(mLocation, EERR_INVALID_VALUE, "Constant division by zero");
				else
					ival = ileft % iright;
				break;
			case TK_LEFT_SHIFT:
				ival = ileft << iright;
				break;
			case TK_RIGHT_SHIFT:
				ival = ileft >> iright;
				break;
			case TK_BINARY_AND:
				ival = ileft & iright;
				break;
			case TK_BINARY_OR:
				ival = ileft | iright;
				break;
			case TK_BINARY_XOR:
				ival = ileft ^ iright;
				break;
			default:
				ival = 0;
			}

			Expression* ex = new Expression(mLocation, EX_CONSTANT);
			Declaration* dec = new Declaration(mLocation, DT_CONST_INTEGER);
			if (mLeft->mDecValue->mBase->mSize <= 2 && mRight->mDecValue->mBase->mSize <= 2)
				dec->mBase = ival < 32768 ? TheSignedIntTypeDeclaration : TheUnsignedIntTypeDeclaration;
			else
				dec->mBase = ival < 2147483648 ? TheSignedLongTypeDeclaration : TheUnsignedLongTypeDeclaration;
			dec->mInteger = ival;
			ex->mDecValue = dec;
			ex->mDecType = dec->mBase;
			return ex;
		}
		else if ((mLeft->mDecValue->mType == DT_CONST_INTEGER || mLeft->mDecValue->mType == DT_CONST_FLOAT) && (mRight->mDecValue->mType == DT_CONST_INTEGER || mRight->mDecValue->mType == DT_CONST_FLOAT))
		{

			double	dval;
			double	dleft = mLeft->mDecValue->mType == DT_CONST_INTEGER ? mLeft->mDecValue->mInteger : mLeft->mDecValue->mNumber;
			double	dright = mRight->mDecValue->mType == DT_CONST_INTEGER ? mRight->mDecValue->mInteger : mRight->mDecValue->mNumber;

			switch (mToken)
			{
			case TK_ADD:
				dval = dleft + dright;
				break;
			case TK_SUB:
				dval = dleft - dright;
				break;
			case TK_MUL:
				dval = dleft * dright;
				break;
			case TK_DIV:
				dval = dleft / dright;
				break;
			default:
				dval = 0;
			}

			Expression* ex = new Expression(mLocation, EX_CONSTANT);
			Declaration* dec = new Declaration(mLocation, DT_CONST_FLOAT);
			dec->mBase = TheFloatTypeDeclaration;
			dec->mNumber = dval;
			ex->mDecValue = dec;
			ex->mDecType = dec->mBase;
			return ex;
		}
		else if (mLeft->mDecValue->mType == DT_CONST_ADDRESS && mRight->mDecValue->mType == DT_CONST_INTEGER)
		{
			int64	ival = 0, ileft = mLeft->mDecValue->mInteger, iright = mRight->mDecValue->mInteger;

			switch (mToken)
			{
			case TK_ADD:
				ival = ileft + iright;
				break;
			case TK_SUB:
				ival = ileft - iright;
				break;
			default:
				return this;
			}

			Expression* ex = new Expression(mLocation, EX_CONSTANT);
			Declaration* dec = new Declaration(mLocation, DT_CONST_ADDRESS);
			dec->mBase = mLeft->mDecType;
			dec->mInteger = ival;
			ex->mDecValue = dec;
			ex->mDecType = dec->mBase;
			return ex;
		}
#if 0
		else if (mLeft->mDecValue->mType == DT_CONST_POINTER && mRight->mDecValue->mType == DT_CONST_INTEGER && (mToken == TK_ADD || mToken == TK_SUB))
		{
			int64	ileft = 0;

			Declaration* pdec = mLeft->mDecValue->mValue->mDecValue;
			if (pdec->mType == DT_VARIABLE_REF)
			{
				ileft = pdec->mOffset;
				pdec = pdec->mBase;
			}

			switch (mToken)
			{
			case TK_ADD:
				ileft += mRight->mDecValue->mInteger * mLeft->mDecType->mBase->mSize;
				break;
			case TK_SUB:
				ileft -= mRight->mDecValue->mInteger * mLeft->mDecType->mBase->mSize;
				break;
			}

			Expression* vex = new Expression(mLocation, EX_VARIABLE);
			Declaration* vdec = new Declaration(mLocation, DT_VARIABLE_REF);
			vdec->mFlags = pdec->mFlags;
			vdec->mBase = pdec;
			vdec->mSize = pdec->mBase->mSize;			
			vdec->mOffset = ileft;
			vex->mDecValue = vdec;
			vex->mDecType = mLeft->mDecType->mBase;

			Expression* ex = new Expression(mLocation, EX_CONSTANT);
			Declaration* dec = new Declaration(mLocation, DT_CONST_POINTER);
			dec->mBase = mLeft->mDecValue->mBase;
			dec->mValue = vex;
			ex->mDecValue = dec;
			ex->mDecType = dec->mBase;
			return ex;
		}
#endif
	}
	else if (mType == EX_RELATIONAL && mLeft->mType == EX_CONSTANT && mRight->mType == EX_CONSTANT)
	{
		if (mLeft->mDecValue->mType == DT_CONST_INTEGER && mRight->mDecValue->mType == DT_CONST_INTEGER)
		{
			int64	ival = 0, ileft = mLeft->mDecValue->mInteger, iright = mRight->mDecValue->mInteger;

			bool	check = false;
			switch (mToken)
			{
			case TK_EQUAL:
				check = ileft == iright;
				break;
			case TK_NOT_EQUAL:
				check = ileft != iright;
				break;
			case TK_GREATER_THAN:
				check = ileft > iright;
				break;
			case TK_GREATER_EQUAL:
				check = ileft >= iright;
				break;
			case TK_LESS_THAN:
				check = ileft < iright;
				break;
			case TK_LESS_EQUAL:
				check = ileft <= iright;
				break;
			}

			Declaration	*	dec = new Declaration(mLocation, DT_CONST_INTEGER);
			dec->mBase = TheBoolTypeDeclaration;
			dec->mInteger = check ? 1 : 0;
			Expression	*	exp = new Expression(mLocation, EX_CONSTANT);
			exp->mDecValue = dec;
			exp->mDecType = dec->mBase;
			return exp;

		}
	}
	else if (mType == EX_CONDITIONAL && mLeft->mType == EX_CONSTANT)
	{
		if (mLeft->mDecValue->mType == DT_CONST_INTEGER)
		{
			if (mLeft->mDecValue->mInteger != 0)
				return mRight->mLeft->ConstantFold(errors);
			else
				return mRight->mRight->ConstantFold(errors);
		}
	}
	else if (mType == EX_BINARY && mToken == TK_ADD && mLeft->mType == EX_VARIABLE && mLeft->mDecValue->mType == DT_VARIABLE && (mLeft->mDecValue->mFlags & DTF_GLOBAL) && mLeft->mDecType->mType == DT_TYPE_ARRAY && mRight->mType == EX_CONSTANT && mRight->mDecValue->mType == DT_CONST_INTEGER)
	{
		Expression* ex = new Expression(mLocation, EX_VARIABLE);
		Declaration* dec = new Declaration(mLocation, DT_VARIABLE_REF);
		dec->mFlags = mLeft->mDecValue->mFlags;
		dec->mBase = mLeft->mDecValue;
		dec->mSize = mLeft->mDecType->mBase->mSize - int(mRight->mDecValue->mInteger) * dec->mSize;
		dec->mOffset = int(mRight->mDecValue->mInteger) * dec->mSize;
		ex->mDecValue = dec;
		ex->mDecType = mLeft->mDecType;
		return ex;
	}
	else if (mType == EX_BINARY && mToken == TK_ADD && mLeft->mType == EX_VARIABLE && mLeft->mDecValue->mType == DT_VARIABLE_REF && (mLeft->mDecValue->mFlags & DTF_GLOBAL) && mLeft->mDecType->mType == DT_TYPE_ARRAY && mRight->mType == EX_CONSTANT && mRight->mDecValue->mType == DT_CONST_INTEGER)
	{
		Expression* ex = new Expression(mLocation, EX_VARIABLE);
		Declaration* dec = new Declaration(mLocation, DT_VARIABLE_REF);
		dec->mFlags = mLeft->mDecValue->mFlags;
		dec->mBase = mLeft->mDecValue->mBase;
		dec->mSize = mLeft->mDecType->mBase->mSize - int(mRight->mDecValue->mInteger) * dec->mSize;
		dec->mOffset = mLeft->mDecValue->mOffset + int(mRight->mDecValue->mInteger) * dec->mSize;
		ex->mDecValue = dec;
		ex->mDecType = mLeft->mDecType;
		return ex;
	}
	else if (mType == EX_BINARY && mToken == TK_ADD && mLeft->mType == EX_VARIABLE && mLeft->mDecValue->mType == DT_VARIABLE && (mLeft->mDecValue->mFlags & DTF_CONST) && mLeft->mDecType->mType == DT_TYPE_POINTER && mRight->mType == EX_CONSTANT && mRight->mDecValue->mType == DT_CONST_INTEGER)
	{
		mLeft = mLeft->mDecValue->mValue;
		return this->ConstantFold(errors);
	}
	else if (mType == EX_QUALIFY && mLeft->mType == EX_VARIABLE && mLeft->mDecValue->mType == DT_VARIABLE && (mLeft->mDecValue->mFlags & DTF_GLOBAL) && mLeft->mDecType->mType == DT_TYPE_STRUCT)
	{
		Expression* ex = new Expression(mLocation, EX_VARIABLE);
		Declaration* dec = new Declaration(mLocation, DT_VARIABLE_REF);
		dec->mFlags = mLeft->mDecValue->mFlags;
		dec->mBase = mLeft->mDecValue;
		dec->mOffset = mDecValue->mOffset;
		dec->mSize = mDecValue->mSize;
		ex->mDecValue = dec;
		ex->mDecType = mDecType;
		return ex;
	}
	else if (mType == EX_QUALIFY && mLeft->mType == EX_VARIABLE && mLeft->mDecValue->mType == DT_VARIABLE_REF && (mLeft->mDecValue->mFlags & DTF_GLOBAL) && mLeft->mDecType->mType == DT_TYPE_STRUCT)
	{
		Expression* ex = new Expression(mLocation, EX_VARIABLE);
		Declaration* dec = new Declaration(mLocation, DT_VARIABLE_REF);
		dec->mFlags = mLeft->mDecValue->mFlags;
		dec->mBase = mLeft->mDecValue->mBase;
		dec->mOffset = mLeft->mDecValue->mOffset + mDecValue->mOffset;
		dec->mSize = mDecValue->mSize;
		ex->mDecValue = dec;
		ex->mDecType = mDecType;
		return ex;
	}

	return this;
}

Declaration::Declaration(const Location& loc, DecType type)
	: mLocation(loc), mEndLocation(loc), mType(type), mScope(nullptr), mData(nullptr), mIdent(nullptr), mQualIdent(nullptr),
	mSize(0), mOffset(0), mFlags(0), mComplexity(0), mLocalSize(0),
	mBase(nullptr), mParams(nullptr), mValue(nullptr), mNext(nullptr), mPrev(nullptr), 
	mConst(nullptr), mMutable(nullptr),
	mDefaultConstructor(nullptr), mDestructor(nullptr), mCopyConstructor(nullptr), mCopyAssignment(nullptr),
	mVectorConstructor(nullptr), mVectorDestructor(nullptr), mVectorCopyConstructor(nullptr), mVectorCopyAssignment(nullptr),
	mVTable(nullptr),
	mVarIndex(-1), mLinkerObject(nullptr), mCallers(nullptr), mCalled(nullptr), mAlignment(1),
	mInteger(0), mNumber(0), mMinValue(-0x80000000LL), mMaxValue(0x7fffffffLL), mFastCallBase(0), mFastCallSize(0), mStride(0), mStripe(1),
	mCompilerOptions(0), mUseCount(0)
{}

Declaration::~Declaration(void)
{
	delete mScope;
	delete[] mData;
}

int Declaration::Stride(void) const
{
	return mStride > 0 ? mStride : mBase->mSize;
}

Declaration* Declaration::BuildConstPointer(const Location& loc)
{
	Declaration* pdec = new Declaration(loc, DT_TYPE_POINTER);
	pdec->mBase = this;
	pdec->mFlags = DTF_DEFINED | DTF_CONST;
	pdec->mSize = 2;
	return pdec;
}

Declaration* Declaration::BuildConstReference(const Location& loc)
{
	Declaration* pdec = new Declaration(loc, DT_TYPE_REFERENCE);
	pdec->mBase = this;
	pdec->mFlags = DTF_DEFINED | DTF_CONST;
	pdec->mSize = 2;
	return pdec;
}

Declaration* Declaration::BuildPointer(const Location& loc)
{
	Declaration* pdec = new Declaration(loc, DT_TYPE_POINTER);
	pdec->mBase = this;
	pdec->mFlags = DTF_DEFINED;
	pdec->mSize = 2;
	return pdec;
}

Declaration* Declaration::BuildReference(const Location& loc)
{
	Declaration* pdec = new Declaration(loc, DT_TYPE_REFERENCE);
	pdec->mBase = this;
	pdec->mFlags = DTF_DEFINED;
	pdec->mSize = 2;
	return pdec;
}

Declaration* Declaration::Last(void)
{
	mPrev = nullptr;
	Declaration* p = this;
	while (p->mNext)
	{
		p->mNext->mPrev = p;
		p = p->mNext;
	}
	return p;
}

Declaration* Declaration::Clone(void)
{
	Declaration* ndec = new Declaration(mLocation, mType);
	ndec->mSize = mSize;
	ndec->mOffset = mOffset;
	ndec->mStride = mStride;
	ndec->mStripe = mStripe;
	ndec->mBase = mBase;
	ndec->mFlags = mFlags;
	ndec->mScope = mScope;
	ndec->mParams = mParams;
	ndec->mIdent = mIdent;
	ndec->mQualIdent = mQualIdent;
	ndec->mValue = mValue;
	ndec->mVarIndex = mVarIndex;
	ndec->mLinkerObject = mLinkerObject;
	ndec->mAlignment = mAlignment;
	ndec->mSection = mSection;
	ndec->mVTable = mVTable;

	return ndec;
}

Declaration* Declaration::ToStriped(Errors * errors)
{
	Declaration* ndec = this->Clone();

	if (mType == DT_TYPE_ARRAY)
	{
		if (mSize == 0)
			errors->Error(ndec->mLocation, ERRR_STRIPE_REQUIRES_FIXED_SIZE_ARRAY, "__striped requires fixed size array");

		ndec->mFlags |= DTF_STRIPED;
		if (mBase->mType == DT_TYPE_ARRAY)
		{
			ndec->mBase = mBase->Clone();
			if (mBase->mSize)
			{
				ndec->mStride = mBase->mSize / mBase->mBase->mSize;
				ndec->mBase->mStride = 1;
				ndec->mBase->mBase = mBase->mBase->ToStriped(mSize / mBase->mBase->mSize);
			}
			else
				errors->Error(ndec->mLocation, ERRR_STRIPE_REQUIRES_FIXED_SIZE_ARRAY, "__striped with zero size");

		}
		else
		{
			ndec->mStride = 1;
			ndec->mBase = mBase->ToStriped(mSize / mBase->mSize);
		}
	}
	else if (ndec->mBase)
	{
		ndec->mBase = mBase->ToStriped(errors);
	}
	else
		errors->Error(ndec->mLocation, ERRR_STRIPE_REQUIRES_FIXED_SIZE_ARRAY, "__striped requires fixed size array");

	return ndec;
}

Declaration* Declaration::ToStriped(int stripe)
{
	Declaration* ndec = new Declaration(mLocation, mType);
	ndec->mSize = mSize;
	ndec->mOffset = mOffset * stripe;
	ndec->mStride = mStride;
	ndec->mStripe = stripe;
	ndec->mFlags = mFlags;
	ndec->mIdent = mIdent;
	ndec->mQualIdent = mQualIdent;

	if (mType == DT_ELEMENT)
		ndec->mBase = mBase->ToStriped(stripe);
	else
		ndec->mBase = mBase;

	if (mType == DT_TYPE_STRUCT)
	{
		ndec->mScope = new DeclarationScope(nullptr, mScope->mLevel);
		Declaration	* p = mParams;
		Declaration* prev = nullptr;
		while (p)
		{
			Declaration* pnec = p->ToStriped(stripe);

			ndec->mScope->Insert(pnec->mIdent, pnec);

			if (prev)
				prev->mNext = pnec;
			else
				ndec->mParams = pnec;
			prev = pnec;
			p = p->mNext;
		}		
	}
	else if (mType == DT_TYPE_FUNCTION)
	{
		ndec->mParams = mParams;
	}
	else if (mType == DT_TYPE_ARRAY)
	{
		ndec->mStride = stripe;
		ndec->mBase = mBase->ToStriped(stripe);
	}
	else
	{
		ndec->mScope = mScope;
		ndec->mParams = mParams;
	}

	return ndec;
}

Declaration* Declaration::ToConstType(void)
{
	if (mFlags & DTF_CONST)
		return this;	

	if (!mConst)
	{
		Declaration* ndec = new Declaration(mLocation, mType);
		ndec->mSize = mSize;
		ndec->mStride = mStride;
		ndec->mBase = mBase;
		ndec->mFlags = mFlags | DTF_CONST;
		ndec->mScope = mScope;
		ndec->mParams = mParams;
		ndec->mIdent = mIdent;
		ndec->mQualIdent = mQualIdent;

		ndec->mDefaultConstructor = mDefaultConstructor;
		ndec->mCopyConstructor = mCopyConstructor;
		ndec->mVectorConstructor = mVectorConstructor;
		ndec->mVectorCopyConstructor = mVectorCopyConstructor;
		ndec->mVTable = mVTable;

		ndec->mMutable = this;
		mConst = ndec;
	}
	
	return mConst;
}

Declaration* Declaration::ToMutableType(void)
{
	if (!(mFlags & DTF_CONST))
		return this;

	if (!mMutable)
	{
		Declaration* ndec = new Declaration(mLocation, mType);
		ndec->mSize = mSize;
		ndec->mStride = mStride;
		ndec->mBase = mBase;
		ndec->mFlags = mFlags | DTF_CONST;
		ndec->mScope = mScope;
		ndec->mParams = mParams;
		ndec->mIdent = mIdent;
		ndec->mQualIdent = mQualIdent;

		ndec->mDefaultConstructor = mDefaultConstructor;
		ndec->mCopyConstructor = mCopyConstructor;
		ndec->mVectorConstructor = mVectorConstructor;
		ndec->mVectorCopyConstructor = mVectorCopyConstructor;
		ndec->mVTable = mVTable;

		ndec->mConst = this;
		mMutable = ndec;
	}

	return mMutable;
}


bool Declaration::IsSubType(const Declaration* dec) const
{
	if (this == dec)
		return true;

	if (mType == DT_TYPE_REFERENCE && dec->mType == DT_TYPE_REFERENCE)
		return mBase->IsSubType(dec->mBase);

	if (mType == DT_TYPE_POINTER || mType == DT_TYPE_ARRAY)
	{
		if (dec->mType == DT_TYPE_POINTER)
			return /*this->Stride() == dec->Stride() &&*/ mBase->IsSubType(dec->mBase);
	}

	if (mType != dec->mType)
		return false;
	if (mType != DT_TYPE_STRUCT && mSize != dec->mSize)
		return false;
	if (mStripe != dec->mStripe)
		return false;

	if ((mFlags & DTF_SIGNED) != (dec->mFlags & DTF_SIGNED))
		return false;

	if ((dec->mFlags & ~mFlags) & (DTF_CONST | DTF_VOLATILE))
		return false;

	if (mType == DT_TYPE_INTEGER)
		return true;
	else if (mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_VOID)
		return true;
	else if (mType == DT_TYPE_STRUCT || mType == DT_TYPE_ENUM)
	{
		if (mScope == dec->mScope || (mIdent == dec->mIdent && mSize == dec->mSize))
			return true;

		if (dec->mBase)
		{
			Declaration* bcdec = dec->mBase;
			while (bcdec)
			{
				if (IsSubType(bcdec->mBase))
					return true;
				bcdec = bcdec->mNext;
			}
		}

		return false;
	}
	else if (mType == DT_TYPE_UNION)
		return false;
	else if (mType == DT_TYPE_ARRAY)
		return mSize <= dec->mSize && mBase->IsSubType(dec->mBase);
	else if (mType == DT_TYPE_FUNCTION)
	{
		if (!dec->mBase->IsSubType(mBase))
			return false;
		Declaration* dl = mParams, * dr = dec->mParams;
		while (dl && dr)
		{
			if (!dl->mBase->IsSubType(dr->mBase))
				return false;
			dl = dl->mNext;
			dr = dr->mNext;
		}

		if (dl || dr)
			return false;

		if ((mFlags & DTF_INTERRUPT) && !(dec->mFlags & DTF_INTERRUPT))
			return false;

		if ((mFlags & DTF_VARIADIC) != (dec->mFlags & DTF_VARIADIC))
			return false;

		return true;
	}

	return false;
}

bool Declaration::IsSameMutable(const Declaration* dec) const
{
	if (this == dec)
		return true;
	if (mType != dec->mType)
		return false;
	if (mSize != dec->mSize)
		return false;
	if (mStripe != dec->mStripe)
		return false;

	if ((mFlags & DTF_SIGNED) != (dec->mFlags & DTF_SIGNED))
		return false;
	if ((dec->mFlags & DTF_CONST) && !(mFlags & DTF_CONST))
		return false;

	if (mType == DT_TYPE_INTEGER)
		return true;
	else if (mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_VOID)
		return true;
	else if (mType == DT_TYPE_ENUM)
		return mIdent == dec->mIdent;
	else if (mType == DT_TYPE_POINTER || mType == DT_TYPE_ARRAY)
		return this->Stride() == dec->Stride() && mBase->IsSame(dec->mBase);
	else if (mType == DT_TYPE_STRUCT)
		return mScope == dec->mScope || (mIdent == dec->mIdent && mSize == dec->mSize);
	else if (mType == DT_TYPE_FUNCTION)
	{
		if (!mBase->IsSame(dec->mBase))
			return false;
		Declaration* dl = mParams, * dr = dec->mParams;
		while (dl && dr)
		{
			if (!dl->mBase->IsSame(dr->mBase))
				return false;
			dl = dl->mNext;
			dr = dr->mNext;
		}

		if (dl || dr)
			return false;

		if ((mFlags & DTF_VARIADIC) != (dec->mFlags & DTF_VARIADIC))
			return false;

		return true;
	}

	return false;
}

bool Declaration::IsConstSame(const Declaration* dec) const
{
	if (this == dec)
		return true;
	if (mType != dec->mType)
		return false;
	if (mSize != dec->mSize)
		return false;
	if (mStripe != dec->mStripe)
		return false;

	if ((mFlags & DTF_SIGNED) != (dec->mFlags & DTF_SIGNED))
		return false;

	if (mType == DT_TYPE_INTEGER)
		return true;
	else if (mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_VOID)
		return true;
	else if (mType == DT_TYPE_ENUM)
		return mIdent == dec->mIdent;
	else if (mType == DT_TYPE_POINTER || mType == DT_TYPE_ARRAY)
		return this->Stride() == dec->Stride() && mBase->IsSame(dec->mBase);
	else if (mType == DT_TYPE_STRUCT)
		return mScope == dec->mScope || (mIdent == dec->mIdent && mSize == dec->mSize);
	else if (mType == DT_TYPE_FUNCTION)
	{
		if (!mBase->IsSame(dec->mBase))
			return false;
		Declaration* dl = mParams, * dr = dec->mParams;
		while (dl && dr)
		{
			if (!dl->mBase->IsSame(dr->mBase))
				return false;
			dl = dl->mNext;
			dr = dr->mNext;
		}

		if (dl || dr)
			return false;

		if ((mFlags & DTF_VARIADIC) != (dec->mFlags & DTF_VARIADIC))
			return false;

		return true;
	}

	return false;
}

bool Declaration::IsSameParams(const Declaration* dec) const
{
	if (mType == DT_TYPE_FUNCTION && dec->mType == DT_TYPE_FUNCTION)
	{
		Declaration* ld = mParams, * rd = dec->mParams;
		while (ld && rd)
		{
			if (!ld->mBase->IsSame(rd->mBase))
				return false;
			ld = ld->mNext;
			rd = rd->mNext;
		}

		return !ld && !rd;
	}
	else
		return false;
}

bool Declaration::IsDerivedFrom(const Declaration* dec) const
{
	if (mType != DT_TYPE_FUNCTION || dec->mType != DT_TYPE_FUNCTION)
		return false;

	if (!(mFlags & DTF_FUNC_THIS) || !(dec->mFlags & DTF_FUNC_THIS))
		return false;

	if (!mBase->IsSame(dec->mBase))
		return false;
	Declaration* dl = mParams->mNext, * dr = dec->mParams->mNext;
	while (dl && dr)
	{
		if (!dl->mBase->IsSame(dr->mBase))
			return false;
		dl = dl->mNext;
		dr = dr->mNext;
	}

	if (dl || dr)
		return false;

	return true;
}

bool Declaration::IsSame(const Declaration* dec) const
{
	if (this == dec)
		return true;
	if (mType != dec->mType)
		return false;
	if (mSize != dec->mSize)
		return false;
	if (mStripe != dec->mStripe)
		return false;

	if ((mFlags & (DTF_SIGNED | DTF_CONST | DTF_VOLATILE)) != (dec->mFlags & (DTF_SIGNED | DTF_CONST | DTF_VOLATILE)))
		return false;

	if (mType == DT_TYPE_INTEGER)
		return true;
	else if (mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_VOID)
		return true;
	else if (mType == DT_TYPE_ENUM)
		return mIdent == dec->mIdent;
	else if (mType == DT_TYPE_POINTER || mType == DT_TYPE_ARRAY)
	{
		if (mBase->mType == DT_TYPE_STRUCT && dec->mBase->mType == DT_TYPE_STRUCT)
		{
			if (mBase->mQualIdent == dec->mBase->mQualIdent &&
				(mBase->mFlags & (DTF_CONST | DTF_VOLATILE)) == (dec->mBase->mFlags & (DTF_CONST | DTF_VOLATILE)))
				return true;
			else
				return false;
		}
		else
			return this->Stride() == dec->Stride() && mBase->IsSame(dec->mBase);
	}
	else if (mType == DT_TYPE_REFERENCE)
		return mBase->IsSame(dec->mBase);
	else if (mType == DT_TYPE_STRUCT)
		return mScope == dec->mScope || (mIdent == dec->mIdent && mSize == dec->mSize);
	else if (mType == DT_TYPE_FUNCTION)
	{
		if (!mBase->IsSame(dec->mBase))
			return false;
		Declaration* dl = mParams, * dr = dec->mParams;
		while (dl && dr)
		{
			if (!dl->mBase->IsSame(dr->mBase))
				return false;
			dl = dl->mNext;
			dr = dr->mNext;
		}

		if (dl || dr)
			return false;

		if ((mFlags & DTF_VARIADIC) != (dec->mFlags & DTF_VARIADIC))
			return false;

		return true;
	}

	return false;
}

bool Declaration::IsSameValue(const Declaration* dec) const 
{
	if (mType != dec->mType || mSize != dec->mSize)
		return false;

	switch (mType)
	{
	case DT_CONST_INTEGER:
		return mInteger == dec->mInteger;
	case DT_CONST_FLOAT:
		return mNumber == dec->mNumber;
	case DT_CONST_ADDRESS:
		return mInteger == dec->mInteger;
	case DT_CONST_POINTER:
		return mValue && dec->mValue && mValue->mType == dec->mValue->mType && mValue->mDecValue == dec->mValue->mDecValue;
	}

	return false;
}

bool Declaration::CanAssign(const Declaration* fromType) const
{
	if (fromType->mType == DT_TYPE_REFERENCE)
		return this->CanAssign(fromType->mBase);
	if (mType == DT_TYPE_REFERENCE)
		return mBase->IsSubType(fromType);

	if (this->IsSame(fromType))
		return true;
	else if (IsNumericType())
	{
		if (fromType->IsNumericType())
			return true;
	}
	else if (mType == DT_TYPE_STRUCT && fromType->mType == DT_TYPE_STRUCT)
	{
		if (mScope == fromType->mScope || (mIdent == fromType->mIdent && mSize == fromType->mSize))
			return true;
		if (fromType->mBase)
			return this->CanAssign(fromType->mBase);
		return false;
	}
	else if (mType == DT_TYPE_POINTER)
	{
		if (fromType->mType == DT_TYPE_POINTER || fromType->mType == DT_TYPE_ARRAY)
		{
			if (mBase->mType == DT_TYPE_VOID || fromType->mBase->mType == DT_TYPE_VOID)
				return (mBase->mFlags & DTF_CONST) || !(fromType->mBase->mFlags & DTF_CONST);
			else if (mBase->IsSubType(fromType->mBase))
				return true;
		}
		else if (mBase->mType == DT_TYPE_FUNCTION && fromType->mType == DT_TYPE_FUNCTION)
		{
			return mBase->IsSubType(fromType);
		}
		else if (mBase->mType == DT_TYPE_VOID && fromType->mType == DT_TYPE_ASSEMBLER)
		{
			return true;
		}
		else if (mBase->mType == DT_TYPE_VOID && fromType->mType == DT_TYPE_FUNCTION)
		{
			return true;
		}
	}

	return false;
}

bool Declaration::IsIntegerType(void) const
{
	return mType == DT_TYPE_INTEGER || mType == DT_TYPE_BOOL || mType == DT_TYPE_ENUM;
}

bool Declaration::IsNumericType(void) const
{
	return mType == DT_TYPE_INTEGER || mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_ENUM;
}


bool Declaration::IsSimpleType(void) const
{
	return mType == DT_TYPE_INTEGER || mType == DT_TYPE_BOOL || mType == DT_TYPE_FLOAT || mType == DT_TYPE_ENUM || mType == DT_TYPE_POINTER;
}

void Declaration::SetDefined(void)
{
	mFlags |= DTF_DEFINED;
	if (mConst)
		mConst->mFlags |= DTF_DEFINED;
}

Declaration* TheVoidTypeDeclaration, * TheConstVoidTypeDeclaration, * TheSignedIntTypeDeclaration, * TheUnsignedIntTypeDeclaration, * TheConstCharTypeDeclaration, * TheCharTypeDeclaration, * TheSignedCharTypeDeclaration, * TheUnsignedCharTypeDeclaration;
Declaration* TheBoolTypeDeclaration, * TheFloatTypeDeclaration, * TheConstVoidPointerTypeDeclaration, * TheVoidPointerTypeDeclaration, * TheSignedLongTypeDeclaration, * TheUnsignedLongTypeDeclaration;
Declaration* TheVoidFunctionTypeDeclaration, * TheConstVoidValueDeclaration;
Declaration* TheCharPointerTypeDeclaration, * TheConstCharPointerTypeDeclaration;

void InitDeclarations(void)
{
	static Location	noloc;
	TheVoidTypeDeclaration = new Declaration(noloc, DT_TYPE_VOID);
	TheVoidTypeDeclaration->mFlags = DTF_DEFINED;

	TheConstVoidTypeDeclaration = new Declaration(noloc, DT_TYPE_VOID);
	TheConstVoidTypeDeclaration->mFlags = DTF_DEFINED | DTF_CONST;

	TheVoidPointerTypeDeclaration = new Declaration(noloc, DT_TYPE_POINTER);
	TheVoidPointerTypeDeclaration->mBase = TheVoidTypeDeclaration;
	TheVoidPointerTypeDeclaration->mSize = 2;
	TheVoidPointerTypeDeclaration->mFlags = DTF_DEFINED;

	TheConstVoidPointerTypeDeclaration = new Declaration(noloc, DT_TYPE_POINTER);
	TheConstVoidPointerTypeDeclaration->mBase = TheConstVoidTypeDeclaration;
	TheConstVoidPointerTypeDeclaration->mSize = 2;
	TheConstVoidPointerTypeDeclaration->mFlags = DTF_DEFINED;

	TheVoidFunctionTypeDeclaration = new Declaration(noloc, DT_TYPE_FUNCTION);
	TheVoidFunctionTypeDeclaration->mBase = TheVoidTypeDeclaration;
	TheVoidFunctionTypeDeclaration->mSize = 2;
	TheVoidFunctionTypeDeclaration->mFlags = DTF_DEFINED;

	TheConstVoidValueDeclaration = new Declaration(noloc, DT_CONST_INTEGER);
	TheVoidFunctionTypeDeclaration->mBase = TheVoidTypeDeclaration;
	TheVoidFunctionTypeDeclaration->mSize = 2;
	TheVoidFunctionTypeDeclaration->mInteger = 0;
	TheVoidFunctionTypeDeclaration->mFlags = DTF_DEFINED;

	TheSignedIntTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheSignedIntTypeDeclaration->mSize = 2;
	TheSignedIntTypeDeclaration->mFlags = DTF_DEFINED | DTF_SIGNED;

	TheUnsignedIntTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheUnsignedIntTypeDeclaration->mSize = 2;
	TheUnsignedIntTypeDeclaration->mFlags = DTF_DEFINED;

	TheSignedLongTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheSignedLongTypeDeclaration->mSize = 4;
	TheSignedLongTypeDeclaration->mFlags = DTF_DEFINED | DTF_SIGNED;

	TheUnsignedLongTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheUnsignedLongTypeDeclaration->mSize = 4;
	TheUnsignedLongTypeDeclaration->mFlags = DTF_DEFINED;

	TheSignedCharTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheSignedCharTypeDeclaration->mSize = 1;
	TheSignedCharTypeDeclaration->mFlags = DTF_DEFINED | DTF_SIGNED;

	TheConstCharTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheConstCharTypeDeclaration->mSize = 1;
	TheConstCharTypeDeclaration->mFlags = DTF_DEFINED | DTF_CONST;

	TheUnsignedCharTypeDeclaration = new Declaration(noloc, DT_TYPE_INTEGER);
	TheUnsignedCharTypeDeclaration->mSize = 1;
	TheUnsignedCharTypeDeclaration->mFlags = DTF_DEFINED;
	TheCharTypeDeclaration = TheUnsignedCharTypeDeclaration;

	TheBoolTypeDeclaration = new Declaration(noloc, DT_TYPE_BOOL);
	TheBoolTypeDeclaration->mSize = 1;
	TheBoolTypeDeclaration->mFlags = DTF_DEFINED;

	TheFloatTypeDeclaration = new Declaration(noloc, DT_TYPE_FLOAT);
	TheFloatTypeDeclaration->mSize = 4;
	TheFloatTypeDeclaration->mFlags = DTF_DEFINED | DTF_SIGNED;


	TheCharPointerTypeDeclaration = new Declaration(noloc, DT_TYPE_POINTER);
	TheCharPointerTypeDeclaration->mBase = TheCharTypeDeclaration;
	TheCharPointerTypeDeclaration->mSize = 2;
	TheCharPointerTypeDeclaration->mFlags = DTF_DEFINED;

	TheConstCharPointerTypeDeclaration = new Declaration(noloc, DT_TYPE_POINTER);
	TheConstCharPointerTypeDeclaration->mBase = TheConstCharTypeDeclaration;
	TheConstCharPointerTypeDeclaration->mSize = 2;
	TheConstCharPointerTypeDeclaration->mFlags = DTF_DEFINED;
}
