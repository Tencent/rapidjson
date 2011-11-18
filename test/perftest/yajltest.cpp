#include "perftest.h"

#if TEST_YAJL

extern "C" {
#include "yajl/yajl_gen.h"
#include "yajl/yajl_parse.h"
#include "yajl/yajl_tree.h"
};

class Yajl : public PerfTest {
public:
	virtual void SetUp() {
		PerfTest::SetUp();
		root_ = yajl_tree_parse(json_, NULL, 0);
		ASSERT_TRUE(root_ != NULL);
	}

	virtual void TearDown() {
		PerfTest::TearDown();
		yajl_tree_free(root_);
	}

protected:
	yajl_val root_;
};

static int null_null(void *) { return 1; }
static int null_boolean(void *, int) { return 1; }
static int null_integer(void *, long long) { return 1; }
static int null_double(void *, double) { return 1; }
static int null_string(void *, const unsigned char*, size_t) { return 1; }
static int null_start_map(void *) { return 1; }
static int null_map_key(void *, const unsigned char*, size_t) { return 1; }
static int null_end_map(void *) { return 1; }
static int null_start_array(void*) { return 1; }
static int null_end_array(void *) { return 1; }

static yajl_callbacks nullcallbacks = {
	null_null,
	null_boolean,
	null_integer,
	null_double,
	NULL,			// yajl_number(). Here we want to test full-parsing performance.
	null_string,
	null_start_map,
	null_map_key,
	null_end_map,
	null_start_array,
	null_end_array
};

TEST_F(Yajl, yajl_parse_nullcallbacks) {
	for (int i = 0; i < kTrialCount; i++) {
		yajl_handle hand = yajl_alloc(&nullcallbacks, NULL, NULL);
		yajl_status stat = yajl_parse(hand, (unsigned char*)json_, length_ - 1);
		//ASSERT_EQ(yajl_status_ok, stat);
		if (stat != yajl_status_ok) {
			unsigned char * str = yajl_get_error(hand, 1, (unsigned char*)json_, length_ + 1);
			fprintf(stderr, "%s", (const char *) str);
		}
		stat = yajl_complete_parse(hand);
		ASSERT_EQ(yajl_status_ok, stat);
		yajl_free(hand);
	}	
}

TEST_F(Yajl, yajl_tree_parse) {
	for (int i = 0; i < kTrialCount; i++) {
		yajl_val root = yajl_tree_parse(json_, NULL, 0);
		ASSERT_TRUE(root != NULL);
		yajl_tree_free(root);
	}
}

yajl_gen_status GenVal(yajl_gen g, yajl_val v) {
	yajl_gen_status status;
	switch (v->type) {
	case yajl_t_string:	return yajl_gen_string(g, (unsigned char*)v->u.string, strlen(v->u.string));

	case yajl_t_number: 
		{
			char buffer[100];
			char *num = buffer;
			size_t len;
			//if (YAJL_IS_INTEGER(v)) // buggy
			if (v->u.number.flags & YAJL_NUMBER_INT_VALID)
				len = sprintf(num, "%d", YAJL_GET_INTEGER(v));
			//else if (YAJL_IS_DOUBLE(v))	// buggy
			else if (v->u.number.flags & YAJL_NUMBER_DOUBLE_VALID)
				len = sprintf(num, "%g", YAJL_GET_DOUBLE(v));
			else {
				num = YAJL_GET_NUMBER(v);
				len = strlen(buffer);
			}
			return yajl_gen_number(g, num, len);
		}

	case yajl_t_object:
		status = yajl_gen_map_open(g);
		if (status != yajl_gen_status_ok)
			return status;
		
		for (size_t i = 0; i < v->u.object.len; i++) {
			status = yajl_gen_string(g, (unsigned char *)v->u.object.keys[i], strlen(v->u.object.keys[i]));
			if (status != yajl_gen_status_ok)
				return status;
			status = GenVal(g, v->u.object.values[i]);
			if (status != yajl_gen_status_ok)
				return status;
		}
		return yajl_gen_map_close(g);

	case yajl_t_array:
		status = yajl_gen_array_open(g);
		if (status != yajl_gen_status_ok)
			return status;
		
		for (size_t i = 0; i < v->u.array.len; i++) {
			status = GenVal(g, v->u.array.values[i]);
			if (status != yajl_gen_status_ok)
				return status;
		}

		return yajl_gen_array_close(g);

	case yajl_t_true: return yajl_gen_bool(g, 1);
	case yajl_t_false: return yajl_gen_bool(g, 0);
	case yajl_t_null: return yajl_gen_null(g);
	}
	return yajl_gen_in_error_state;
}

TEST_F(Yajl, yajl_gen) {
	for (int i = 0; i < kTrialCount; i++) {
		yajl_gen g = yajl_gen_alloc(NULL);

		yajl_gen_status status = GenVal(g, root_);
		if (status != yajl_gen_status_ok) {
			std::cout << "gen error: " << status << std::endl;
			FAIL();
		}

		const unsigned char * buf;
		size_t len;
		status = yajl_gen_get_buf(g, &buf, &len);
		ASSERT_EQ(yajl_gen_status_ok, status);
		//if (i == 0)
		//	std::cout << len << std::endl;
		yajl_gen_free(g);
	}	
}

TEST_F(Yajl, yajl_gen_beautify) {
	for (int i = 0; i < kTrialCount; i++) {
		yajl_gen g = yajl_gen_alloc(NULL);
		yajl_gen_config(g, yajl_gen_beautify, 1);
		yajl_gen_config(g, yajl_gen_indent_string, " ");

		yajl_gen_status status = GenVal(g, root_);
		if (status != yajl_gen_status_ok) {
			std::cout << "gen error: " << status << std::endl;
			FAIL();
		}

		const unsigned char * buf;
		size_t len;
		status = yajl_gen_get_buf(g, &buf, &len);
		ASSERT_EQ(yajl_gen_status_ok, status);
		//if (i == 0)
		//	std::cout << len << std::endl;
		yajl_gen_free(g);
	}	
}

TEST_F(Yajl, Whitespace) {
	for (int i = 0; i < kTrialCount; i++) {
		yajl_val root = yajl_tree_parse(whitespace_, NULL, 0);
		ASSERT_TRUE(root != NULL);
		yajl_tree_free(root);
	}
}

#endif // TEST_YAJL
