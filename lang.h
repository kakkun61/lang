#ifndef LANG
#define LANG
	
typedef struct ExpressionS Expression;

typedef enum {
	VALUE,
	ADD,
	SUB,
	MUL,
	DIV
} ExpressionType;

typedef enum {
	INTEGER,
	FLOAT,
} ValueType;

typedef struct {
	ValueType type;
	union {
		double float_point;
		int integer;
	} u;
} Value;

typedef struct ExpressionPairS {
	Expression *left;
	Expression *right;
} ExpressionPair;

typedef struct ExpressionListS {
	Expression *expression;
	struct ExpressionListS *next;
} ExpressionList;

struct ExpressionS {
	ExpressionType type;
	union {
		Value *value;
		ExpressionPair *pair;
	} u;
};

Expression *create_expression(ExpressionType type) {
	Expression *expr = malloc(sizeof(Expression));
	expr->type = type;
	return expr;
}

Value *create_value(ValueType type) {
	Value *val = malloc(sizeof(Value));
	val->type = type;
	return val;
}

Value *create_float_point(double value) {
	Value *val = create_value(FLOAT);
	val->u.float_point = value;
	return val;
}

Value *create_integer(int value) {
	Value *val = create_value(INTEGER);
	val->u.integer = value;
	return val;
}

ExpressionPair *create_expression_pair(Expression *left, Expression *right) {
	ExpressionPair *pair = malloc(sizeof(ExpressionPair));
	pair->left = left;
	pair->right = right;
	return pair;
}

Expression *create_binary_expression(ExpressionType type, Expression *left, Expression *right) {
	Expression *expr = create_expression(type);
	expr->u.pair = create_expression_pair(left, right);
	return expr;
}

Expression *create_value_expression(Value *value) {
	Expression *expr = create_expression(VALUE);
	expr->u.value = value;
	return expr;
}

#define BINEXP2(op, left, right) ((left) op (right))
#define BINEXP(op, left, right)\
	switch ((left)->type) {\
	case FLOAT:\
		switch ((right)->type) {\
		case FLOAT:\
			return create_float_point(\
					BINEXP2(op, (left)->u.float_point, (right)->u.float_point)\
			);\
		case INTEGER:\
			return create_float_point(\
					BINEXP2(op, (left)->u.float_point, (right)->u.integer)\
			);\
		}\
	case INTEGER:\
		switch ((right)->type) {\
		case FLOAT:\
			return create_float_point(\
					BINEXP2(op, (left)->u.integer, (right)->u.float_point)\
			);\
		case INTEGER:\
			return create_integer(\
					BINEXP2(op, (left)->u.integer, (right)->u.integer)\
			);\
		}\
	}

Value *eval(Expression *expression) {
	switch (expression->type) {
	case VALUE:
		return expression->u.value;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		{
			Value *left = eval(expression->u.pair->left);
			Value *right = eval(expression->u.pair->right);
			switch (expression->type) {
			case ADD:
				BINEXP(+, left, right);
			case SUB:
				BINEXP(-, left, right);
			case MUL:
				BINEXP(*, left, right);
			case DIV:
				BINEXP(/, left, right);
			}
		}
	}
}

#undef BINEXP2
#undef BINEXP

#endif
