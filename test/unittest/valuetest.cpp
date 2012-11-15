#include "unittest.h"
#include "rapidjson/document.h"

using namespace rapidjson;

TEST(Value, default_constructor) {
	Value x;
	EXPECT_EQ(kNullType, x.GetType());
	EXPECT_TRUE(x.IsNull());

	//std::cout << "sizeof(Value): " << sizeof(x) << std::endl;
}

// Should not pass compilation
//TEST(Value, copy_constructor) {
//	Value x(1234);
//	Value y = x;
//}

TEST(Value, assignment_operator) {
	Value x(1234);
	Value y;
	y = x;
	EXPECT_TRUE(x.IsNull());	// move semantic
	EXPECT_EQ(1234, y.GetInt());
}


TEST(Value, Null) {
	// Default constructor
	Value x;
	EXPECT_EQ(kNullType, x.GetType());
	EXPECT_TRUE(x.IsNull());

	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsNumber());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// Constructor with type
	Value y(kNullType);
	EXPECT_TRUE(y.IsNull());

	// SetNull();
	Value z(true);
	z.SetNull();
	EXPECT_TRUE(z.IsNull());
}

TEST(Value, True) {
	// Constructor with bool
	Value x(true);
	EXPECT_EQ(kTrueType, x.GetType());
	EXPECT_TRUE(x.GetBool());
	EXPECT_TRUE(x.IsBool());
	EXPECT_TRUE(x.IsTrue());

	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsNumber());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// Constructor with type
	Value y(kTrueType);
	EXPECT_TRUE(y.IsTrue());

	// SetBool()
	Value z;
	z.SetBool(true);
	EXPECT_TRUE(z.IsTrue());
}

TEST(Value, False) {
	// Constructor with bool
	Value x(false);
	EXPECT_EQ(kFalseType, x.GetType());
	EXPECT_TRUE(x.IsBool());
	EXPECT_TRUE(x.IsFalse());

	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.GetBool());
	//EXPECT_FALSE((bool)x);
	EXPECT_FALSE(x.IsNumber());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// Constructor with type
	Value y(kFalseType);
	EXPECT_TRUE(y.IsFalse());

	// SetBool()
	Value z;
	z.SetBool(false);
	EXPECT_TRUE(z.IsFalse());
}

TEST(Value, Int) {
	// Constructor with int
	Value x(1234);
	EXPECT_EQ(kNumberType, x.GetType());
	EXPECT_EQ(1234, x.GetInt());
	EXPECT_EQ(1234u, x.GetUint());
	EXPECT_EQ(1234, x.GetInt64());
	EXPECT_EQ(1234u, x.GetUint64());
	EXPECT_EQ(1234, x.GetDouble());
	//EXPECT_EQ(1234, (int)x);
	//EXPECT_EQ(1234, (unsigned)x);
	//EXPECT_EQ(1234, (int64_t)x);
	//EXPECT_EQ(1234, (uint64_t)x);
	//EXPECT_EQ(1234, (double)x);
	EXPECT_TRUE(x.IsNumber());
	EXPECT_TRUE(x.IsInt());
	EXPECT_TRUE(x.IsUint());
	EXPECT_TRUE(x.IsInt64());
	EXPECT_TRUE(x.IsUint64());

	EXPECT_FALSE(x.IsDouble());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	Value nx(-1234);
	EXPECT_EQ(-1234, nx.GetInt());
	EXPECT_EQ(-1234, nx.GetInt64());
	EXPECT_TRUE(nx.IsInt());
	EXPECT_TRUE(nx.IsInt64());
	EXPECT_FALSE(nx.IsUint());
	EXPECT_FALSE(nx.IsUint64());

	// Constructor with type
	Value y(kNumberType);
	EXPECT_TRUE(y.IsNumber());
	EXPECT_TRUE(y.IsInt());
	EXPECT_EQ(0, y.GetInt());

	// SetInt()
	Value z;
	z.SetInt(1234);
	EXPECT_EQ(1234, z.GetInt());

	// operator=(int)
	z = 5678;
	EXPECT_EQ(5678, z.GetInt());
}

TEST(Value, Uint) {
	// Constructor with int
	Value x(1234u);
	EXPECT_EQ(kNumberType, x.GetType());
	EXPECT_EQ(1234, x.GetInt());
	EXPECT_EQ(1234u, x.GetUint());
	EXPECT_EQ(1234, x.GetInt64());
	EXPECT_EQ(1234u, x.GetUint64());
	EXPECT_TRUE(x.IsNumber());
	EXPECT_TRUE(x.IsInt());
	EXPECT_TRUE(x.IsUint());
	EXPECT_TRUE(x.IsInt64());
	EXPECT_TRUE(x.IsUint64());
	EXPECT_EQ(1234.0, x.GetDouble());	// Number can always be cast as double but !IsDouble().

	EXPECT_FALSE(x.IsDouble());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// SetUint()
	Value z;
	z.SetUint(1234);
	EXPECT_EQ(1234u, z.GetUint());

	// operator=(unsigned)
	z = 5678u;
	EXPECT_EQ(5678u, z.GetUint());

	z = 2147483648u;	// 2^31, cannot cast as int
	EXPECT_EQ(2147483648u, z.GetUint());
	EXPECT_FALSE(z.IsInt());
	EXPECT_TRUE(z.IsInt64());	// Issue 41: Incorrect parsing of unsigned int number types
}

TEST(Value, Int64) {
	// Constructor with int
	Value x(int64_t(1234LL));
	EXPECT_EQ(kNumberType, x.GetType());
	EXPECT_EQ(1234, x.GetInt());
	EXPECT_EQ(1234u, x.GetUint());
	EXPECT_EQ(1234, x.GetInt64());
	EXPECT_EQ(1234u, x.GetUint64());
	EXPECT_TRUE(x.IsNumber());
	EXPECT_TRUE(x.IsInt());
	EXPECT_TRUE(x.IsUint());
	EXPECT_TRUE(x.IsInt64());
	EXPECT_TRUE(x.IsUint64());

	EXPECT_FALSE(x.IsDouble());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	Value nx(int64_t(-1234LL));
	EXPECT_EQ(-1234, nx.GetInt());
	EXPECT_EQ(-1234, nx.GetInt64());
	EXPECT_TRUE(nx.IsInt());
	EXPECT_TRUE(nx.IsInt64());
	EXPECT_FALSE(nx.IsUint());
	EXPECT_FALSE(nx.IsUint64());

	// SetInt64()
	Value z;
	z.SetInt64(1234);
	EXPECT_EQ(1234, z.GetInt64());

	z.SetInt64(2147483648LL);	// 2^31, cannot cast as int
	EXPECT_FALSE(z.IsInt());
	EXPECT_TRUE(z.IsUint());

	z.SetInt64(4294967296LL);	// 2^32, cannot cast as uint
	EXPECT_FALSE(z.IsInt());
	EXPECT_FALSE(z.IsUint());
}

TEST(Value, Uint64) {
	// Constructor with int
	Value x(uint64_t(1234LL));
	EXPECT_EQ(kNumberType, x.GetType());
	EXPECT_EQ(1234, x.GetInt());
	EXPECT_EQ(1234u, x.GetUint());
	EXPECT_EQ(1234, x.GetInt64());
	EXPECT_EQ(1234u, x.GetUint64());
	EXPECT_TRUE(x.IsNumber());
	EXPECT_TRUE(x.IsInt());
	EXPECT_TRUE(x.IsUint());
	EXPECT_TRUE(x.IsInt64());
	EXPECT_TRUE(x.IsUint64());

	EXPECT_FALSE(x.IsDouble());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// SetUint64()
	Value z;
	z.SetUint64(1234);
	EXPECT_EQ(1234u, z.GetUint64());

	z.SetUint64(2147483648LL);	// 2^31, cannot cast as int
	EXPECT_FALSE(z.IsInt());
	EXPECT_TRUE(z.IsUint());
	EXPECT_TRUE(z.IsInt64());

	z.SetUint64(4294967296LL);	// 2^32, cannot cast as uint
	EXPECT_FALSE(z.IsInt());
	EXPECT_FALSE(z.IsUint());
	EXPECT_TRUE(z.IsInt64());

	z.SetUint64(9223372036854775808uLL);	// 2^63 cannot cast as int64
	EXPECT_FALSE(z.IsInt64());

	// Issue 48
	EXPECT_EQ(9223372036854775808uLL, z.GetUint64());
}

TEST(Value, Double) {
	// Constructor with double
	Value x(12.34);
	EXPECT_EQ(kNumberType, x.GetType());
	EXPECT_EQ(12.34, x.GetDouble());
	EXPECT_TRUE(x.IsNumber());
	EXPECT_TRUE(x.IsDouble());

	EXPECT_FALSE(x.IsInt());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// SetDouble()
	Value z;
	z.SetDouble(12.34);
	EXPECT_EQ(12.34, z.GetDouble());

	z = 56.78;
	EXPECT_EQ(56.78, z.GetDouble());
}

TEST(Value, String) {
	// Constructor with const string
	Value x("Hello", 5);
	EXPECT_EQ(kStringType, x.GetType());
	EXPECT_TRUE(x.IsString());
	EXPECT_STREQ("Hello", x.GetString());
	EXPECT_EQ(5u, x.GetStringLength());

	EXPECT_FALSE(x.IsNumber());
	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsObject());
	EXPECT_FALSE(x.IsArray());

	// Constructor with copy string
	MemoryPoolAllocator<> allocator;
	Value c(x.GetString(), x.GetStringLength(), allocator);
	//x.SetString("World");
	x.SetString("World", 5);
	EXPECT_STREQ("Hello", c.GetString());
	EXPECT_EQ(5u, c.GetStringLength());

	// Constructor with type
	Value y(kStringType);
	EXPECT_TRUE(y.IsString());
	EXPECT_EQ(0, y.GetString());
	EXPECT_EQ(0u, y.GetStringLength());

	// SetConsttring()
	Value z;
	//z.SetString("Hello");
	z.SetString("Hello", 5);
	EXPECT_STREQ("Hello", z.GetString());
	EXPECT_EQ(5u, z.GetStringLength());

	// SetString()
	char s[] = "World";
	Value w;
	w.SetString(s, (SizeType)strlen(s), allocator);
	s[0] = '\0';
	EXPECT_STREQ("World", w.GetString());
	EXPECT_EQ(5u, w.GetStringLength());
}

TEST(Value, Array) {
	Value x(kArrayType);
	const Value& y = x;
	Value::AllocatorType allocator;

	EXPECT_EQ(kArrayType, x.GetType());
	EXPECT_TRUE(x.IsArray());
	EXPECT_TRUE(x.Empty());
	EXPECT_EQ(0u, x.Size());
	EXPECT_TRUE(y.IsArray());
	EXPECT_TRUE(y.Empty());
	EXPECT_EQ(0u, y.Size());

	EXPECT_FALSE(x.IsNull());
	EXPECT_FALSE(x.IsBool());
	EXPECT_FALSE(x.IsFalse());
	EXPECT_FALSE(x.IsTrue());
	EXPECT_FALSE(x.IsString());
	EXPECT_FALSE(x.IsObject());

	// PushBack()
	Value v;
	x.PushBack(v, allocator);
	v.SetBool(true);
	x.PushBack(v, allocator);
	v.SetBool(false);
	x.PushBack(v, allocator);
	v.SetInt(123);
	x.PushBack(v, allocator);

	EXPECT_FALSE(x.Empty());
	EXPECT_EQ(4u, x.Size());
	EXPECT_FALSE(y.Empty());
	EXPECT_EQ(4u, y.Size());
	EXPECT_TRUE(x[SizeType(0)].IsNull());
	EXPECT_TRUE(x[1u].IsTrue());
	EXPECT_TRUE(x[2u].IsFalse());
	EXPECT_TRUE(x[3u].IsInt());
	EXPECT_EQ(123, x[3u].GetInt());
	EXPECT_TRUE(y[SizeType(0)].IsNull());
	EXPECT_TRUE(y[1u].IsTrue());
	EXPECT_TRUE(y[2u].IsFalse());
	EXPECT_TRUE(y[3u].IsInt());
	EXPECT_EQ(123, y[3u].GetInt());

	// iterator
	Value::ValueIterator itr = x.Begin();
	EXPECT_TRUE(itr != x.End());
	EXPECT_TRUE(itr->IsNull());
	++itr;
	EXPECT_TRUE(itr != x.End());
	EXPECT_TRUE(itr->IsTrue());
	++itr;
	EXPECT_TRUE(itr != x.End());
	EXPECT_TRUE(itr->IsFalse());
	++itr;
	EXPECT_TRUE(itr != x.End());
	EXPECT_TRUE(itr->IsInt());
	EXPECT_EQ(123, itr->GetInt());

	// const iterator
	Value::ConstValueIterator citr = y.Begin();
	EXPECT_TRUE(citr != y.End());
	EXPECT_TRUE(citr->IsNull());
	++citr;
	EXPECT_TRUE(citr != y.End());
	EXPECT_TRUE(citr->IsTrue());
	++citr;
	EXPECT_TRUE(citr != y.End());
	EXPECT_TRUE(citr->IsFalse());
	++citr;
	EXPECT_TRUE(citr != y.End());
	EXPECT_TRUE(citr->IsInt());
	EXPECT_EQ(123, citr->GetInt());

	// PopBack()
	x.PopBack();
	EXPECT_EQ(3u, x.Size());
	EXPECT_TRUE(y[SizeType(0)].IsNull());
	EXPECT_TRUE(y[1].IsTrue());
	EXPECT_TRUE(y[2].IsFalse());

	// Clear()
	x.Clear();
	EXPECT_TRUE(x.Empty());
	EXPECT_EQ(0u, x.Size());
	EXPECT_TRUE(y.Empty());
	EXPECT_EQ(0u, y.Size());

	// SetArray()
	Value z;
	z.SetArray();
	EXPECT_TRUE(z.IsArray());
	EXPECT_TRUE(z.Empty());
}

TEST(Value, Object) {
	Value x(kObjectType);
	const Value& y = x;	// const version
	Value::AllocatorType allocator;

	EXPECT_EQ(kObjectType, x.GetType());
	EXPECT_TRUE(x.IsObject());
	EXPECT_EQ(kObjectType, y.GetType());
	EXPECT_TRUE(y.IsObject());

	// AddMember()
	Value name("A", 1);
	Value value("Apple", 5);
	x.AddMember(name, value, allocator);
	//name.SetString("B");
	name.SetString("B", 1);
	//value.SetString("Banana");
	value.SetString("Banana", 6);
	x.AddMember(name, value, allocator);

	// HasMember()
	EXPECT_TRUE(x.HasMember("A"));
	EXPECT_TRUE(x.HasMember("B"));
	EXPECT_TRUE(y.HasMember("A"));
	EXPECT_TRUE(y.HasMember("B"));

	// operator[]
	EXPECT_STREQ("Apple", x["A"].GetString());
	EXPECT_STREQ("Banana", x["B"].GetString());

	// const operator[]
	EXPECT_STREQ("Apple", y["A"].GetString());
	EXPECT_STREQ("Banana", y["B"].GetString());

	// member iterator
	Value::MemberIterator itr = x.MemberBegin(); 
	EXPECT_TRUE(itr != x.MemberEnd());
	EXPECT_STREQ("A", itr->name.GetString());
	EXPECT_STREQ("Apple", itr->value.GetString());
	++itr;
	EXPECT_TRUE(itr != x.MemberEnd());
	EXPECT_STREQ("B", itr->name.GetString());
	EXPECT_STREQ("Banana", itr->value.GetString());
	++itr;
	EXPECT_FALSE(itr != x.MemberEnd());

	// const member iterator
	Value::ConstMemberIterator citr = y.MemberBegin(); 
	EXPECT_TRUE(citr != y.MemberEnd());
	EXPECT_STREQ("A", citr->name.GetString());
	EXPECT_STREQ("Apple", citr->value.GetString());
	++citr;
	EXPECT_TRUE(citr != y.MemberEnd());
	EXPECT_STREQ("B", citr->name.GetString());
	EXPECT_STREQ("Banana", citr->value.GetString());
	++citr;
	EXPECT_FALSE(citr != y.MemberEnd());

	// RemoveMember()
	x.RemoveMember("A");
	EXPECT_FALSE(x.HasMember("A"));

	x.RemoveMember("B");
	EXPECT_FALSE(x.HasMember("B"));

	EXPECT_TRUE(x.MemberBegin() == x.MemberEnd());

	// SetObject()
	Value z;
	z.SetObject();
	EXPECT_TRUE(z.IsObject());
}

TEST(Value, BigNestedArray) {
	MemoryPoolAllocator<> allocator;
	Value x(kArrayType);
	static const SizeType  n = 200;

	for (SizeType i = 0; i < n; i++) {
		Value y(kArrayType);
		for (SizeType  j = 0; j < n; j++) {
			Value number((int)(i * n + j));
			y.PushBack(number, allocator);
		}
		x.PushBack(y, allocator);
	}

	for (SizeType i = 0; i < n; i++)
		for (SizeType j = 0; j < n; j++) {
			EXPECT_TRUE(x[i][j].IsInt());
			EXPECT_EQ((int)(i * n + j), x[i][j].GetInt());
		}
}

TEST(Value, BigNestedObject) {
	MemoryPoolAllocator<> allocator;
	Value x(kObjectType);
	static const SizeType n = 200;

	for (SizeType i = 0; i < n; i++) {
		char name1[10];
		sprintf(name1, "%d", i);

		Value name(name1, (SizeType)strlen(name1), allocator);
		Value object(kObjectType);

		for (SizeType j = 0; j < n; j++) {
			char name2[10];
			sprintf(name2, "%d", j);

			Value name(name2, (SizeType)strlen(name2), allocator);
			Value number((int)(i * n + j));
			object.AddMember(name, number, allocator);
		}

		x.AddMember(name, object, allocator);
	}

	for (SizeType i = 0; i < n; i++) {
		char name1[10];
		sprintf(name1, "%d", i);
		
		for (SizeType j = 0; j < n; j++) {
			char name2[10];
			sprintf(name2, "%d", j);
			x[name1];
			EXPECT_EQ((int)(i * n + j), x[name1][name2].GetInt());
		}
	}
}

// Issue 18: Error removing last element of object
// http://code.google.com/p/rapidjson/issues/detail?id=18
TEST(Value, RemoveLastElement) {
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	rapidjson::Value objVal(rapidjson::kObjectType);        
	objVal.AddMember("var1", 123, allocator);       
	objVal.AddMember("var2", "444", allocator);
	objVal.AddMember("var3", 555, allocator);
	EXPECT_TRUE(objVal.HasMember("var3"));
	objVal.RemoveMember("var3");    // Assertion here in r61
	EXPECT_FALSE(objVal.HasMember("var3"));
}

// Issue 38:	Segmentation fault with CrtAllocator
TEST(Document, CrtAllocator) {
	typedef GenericValue<UTF8<>, CrtAllocator> V;

	V::AllocatorType allocator;
	V o(kObjectType);
	o.AddMember("x", 1, allocator);	// Should not call destructor on uninitialized name/value of newly allocated members.

	V a(kArrayType);
	a.PushBack(1, allocator);	// Should not call destructor on uninitialized Value of newly allocated elements.
}
