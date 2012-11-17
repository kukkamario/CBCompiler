#ifndef VALUE_H
#define VALUE_H
class ValueType;

class Value
{
	public:
		Value();
		ValueType *valueType() const {return mValueType;}
	private:
		ValueType *mValueType;
};

#endif // VALUE_H
