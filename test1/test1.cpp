// test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>

template <typename T>
class simple_vector
{
public:
	const size_t element_size = sizeof(T);

	simple_vector()
	{
		buffer = (T*)malloc(16 * element_size);
		buffsize = 16;
		size = 0;
	}

	T* resize_buffer(size_t new_size)
	{
		if (new_size > buffsize)
			buffer = (T*)realloc(buffer, new_size * element_size);
		
		return buffer;
	}

	simple_vector(const T* data, size_t data_size)
		: simple_vector()
	{
		append(data, data_size);
	}

	simple_vector<T>& append(const T* data, size_t data_size)
	{
		resize_buffer(size + data_size);
		memcpy(buffer + size, data, data_size * element_size);
		size += data_size;

		return (*this);
	}

	simple_vector<T>& append(const T& element)
	{
		resize_buffer(size + 1);
		buffer[size] = element;
		size++;

		return (*this);
	}

	simple_vector<T>& append(const simple_vector<T>& other)
	{
		resize_buffer(size + other.size);

		memcpy(buffer + size, other.buffer, other.size * element_size);
		size += other.size;

		return (*this);
	}

	void clear()
	{
		size = 0;
	}

	simple_vector<T>& operator=(const simple_vector<T>& other)
	{
		if (&other == this)
			return (*this);

		buffer = (T*)malloc(other.size * element_size);
		buffsize = other.size;

		memcpy(buffer, other.buffer, other.size * element_size);
		size = other.size;

		return (*this);
	}

	bool operator==(const simple_vector<T>& other)
	{
		if (other.size != size)
			return false;

		size_t i = 0;
		T* it1 = buffer;
		T* it2 = other.buffer;

		while (i < size)
		{
			if ((*it1) != (*it2))
				return false;
			it1++;
			it2++;
		}
		return true;
	}

private:
	T* buffer;
	size_t buffsize;
	size_t size;
};

// not null terminated
class simple_string : public simple_vector<char>
{
public:
	simple_string()
		: simple_vector<char>()
	{}

	simple_string(const char* text)
		: simple_vector<char>()
	{
		size_t l = strlen(text);
		append(text, l);
	}
};

class node
{
public:
	node()
	{}

	simple_string name;
	simple_vector<node> childs;
};

class element_node : public node
{
public:
	element_node(const simple_string& _name)
		: node()
	{
		name = _name;
	}

	simple_string value;
};

class group_node : public node
{
public:
	group_node(const simple_string& _name)
		: node()
	{
		name = _name;
	}

	const element_node& append_child(const element_node& element)
	{
		childs.append(element);
		return element;
	}
};

class root_node : public node
{
public:
	root_node()
	{}

	const group_node& append_child(const group_node& group)
	{
		childs.append(group);
		return group;
	}
};



int main()
{
	root_node root{};

	group_node group1 = root.append_child({ "GROUP1" });
	group_node group2 = root.append_child({ "GROUP2" });

	element_node element1 = group1.append_child({ "element1" });
	element_node element2 = group1.append_child({ "element2" });

	element_node element3 = group2.append_child({ "element3" });
	element_node element4 = group2.append_child({ "element4" });



	return 0;
}
