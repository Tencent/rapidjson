// Serialize example
// This example shows writing JSON string with writer directly.

#include "rapidjson/prettywriter.h"	// for stringify JSON
#include "rapidjson/filestream.h"	// wrapper of C stream for prettywriter as output
#include <cstdio>
#include <string>
#include <vector>

using namespace rapidjson;

class Person {
public:
	Person(const std::string& name, unsigned age) : name_(name), age_(age) {}
	virtual ~Person() {}

protected:
	template <typename Writer>
	void Serialize(Writer& writer) const {
		// This base class just write out name-value pairs, without wrapping within an object.
		writer.String("name");
		writer.String(name_.c_str(), (SizeType)name_.length());	// Suppling length of string is faster.

		writer.String("age");
		writer.Uint(age_);
	}

private:
	std::string name_;
	unsigned age_;
};

class Education {
public:
	Education(const std::string& school, double GPA) : school_(school), GPA_(GPA) {}

	template <typename Writer>
	void Serialize(Writer& writer) const {
		writer.StartObject();
		
		writer.String("school");
		writer.String(school_.c_str(), (SizeType)school_.length());

		writer.String("GPA");
		writer.Double(GPA_);

		writer.EndObject();
	}

private:
	std::string school_;
	double GPA_;
};

class Dependent : public Person {
public:
	Dependent(const std::string& name, unsigned age, Education* education = 0) : Person(name, age), education_(education) {}
	Dependent(const Dependent& rhs) : Person(rhs) { education_ = (rhs.education_ == 0) ? 0 : new Education(*rhs.education_); }
	~Dependent() { delete education_; }

	template <typename Writer>
	void Serialize(Writer& writer) const {
		writer.StartObject();

		Person::Serialize(writer);

		writer.String("education");
		if (education_)
			education_->Serialize(writer);
		else
			writer.Null();

		writer.EndObject();
	}

private:
	Education *education_;
};

class Employee : public Person {
public:
	Employee(const std::string& name, unsigned age, bool married) : Person(name, age), married_(married) {}

	void AddDependent(const Dependent& dependent) {
		dependents_.push_back(dependent);
	}

	template <typename Writer>
	void Serialize(Writer& writer) const {
		writer.StartObject();

		Person::Serialize(writer);

		writer.String("married");
		writer.Bool(married_);

		writer.String(("dependents"));
		writer.StartArray();
		for (std::vector<Dependent>::const_iterator dependentItr = dependents_.begin(); dependentItr != dependents_.end(); ++dependentItr)
			dependentItr->Serialize(writer);
		writer.EndArray();

		writer.EndObject();
	}

private:
	bool married_;
	std::vector<Dependent> dependents_;
};

int main(int, char*[]) {
	std::vector<Employee> employees;

	employees.push_back(Employee("Milo YIP", 34, true));
	employees.back().AddDependent(Dependent("Lua YIP", 3, new Education("Happy Kindergarten", 3.5)));
	employees.back().AddDependent(Dependent("Mio YIP", 1));

	employees.push_back(Employee("Percy TSE", 30, false));

	FileStream s(stdout);
	PrettyWriter<FileStream> writer(s);		// Can also use Writer for condensed formatting

	writer.StartArray();
	for (std::vector<Employee>::const_iterator employeeItr = employees.begin(); employeeItr != employees.end(); ++employeeItr)
		employeeItr->Serialize(writer);
	writer.EndArray();

	return 0;
}
