// test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <type_traits>
#include <iostream>

#define MAX_SIZE_T ((size_t)-1)

template <typename T>
class simple_vector
{
public:

	static T end;

	simple_vector()
	{
		buffer = (T*)malloc(16 * sizeof(T));
		buffsize = 16;
		size = 0;
	}

	T* resize_buffer(size_t new_size) const
	{
		if (new_size > buffsize)
		{
			buffer = (T*)realloc(buffer, new_size * sizeof(T));
			buffsize = new_size;
		}
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
		memcpy(buffer + size, data, data_size * sizeof(T));
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

		memcpy(buffer + size, other.buffer, other.size * sizeof(T));
		size += other.size;

		return (*this);
	}

	void clear()
	{
		if (std::is_class<T>::value)
		{
			for (size_t i = 0; i < size; ++i)
			{
				buffer[i].~T();
			}
		}
		size = 0;
	}

	T& operator[](size_t n)
	{
		if (n >= size)
			return end;

		return buffer[n];
	}

	simple_vector<T>& operator=(const simple_vector<T>& other)
	{
		if (&other == this)
			return (*this);

		buffer = (T*)malloc(other.size * sizeof(T));
		buffsize = other.size;

		memcpy(buffer, other.buffer, other.size * sizeof(T));
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
			i++;
		}
		return true;
	}

	bool operator!=(const simple_vector<T>& other)
	{
		return !(operator==(other));
	}

	size_t get_size()
	{
		return size;
	}

protected:
	mutable T* buffer;
	mutable size_t buffsize;
	mutable size_t size;
};

template <typename T>
T simple_vector<T>::end;

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

	simple_string(const char* text, size_t len)
		: simple_vector<char>()
	{
		append(text, len);
	}

	const char* c_str() const
	{
		resize_buffer(size + 1);
		buffer[size] = 0;
		return buffer;
	}

	size_t find(char c, size_t from = 0)
	{
		for (size_t i = from; i < size; ++i)
		{
			if (buffer[i] == c)
				return i;
		}
		return MAX_SIZE_T;
	}

	bool operator==(const char* other)
	{
		return simple_vector<char>::operator==(simple_string{ other });
	}

	bool operator!=(const char* other)
	{
		return !simple_vector<char>::operator==(simple_string{ other });
	}

	bool operator==(const simple_string& other)
	{
		return simple_vector<char>::operator==(other);
	}

	bool operator!=(const simple_string& other)
	{
		return !simple_vector<char>::operator==(other);
	}

	simple_string substring(size_t from, size_t to = MAX_SIZE_T) const
	{
		if (to > size)
			to = size;

		return { buffer + from, to - from };
	}

	simple_vector<simple_string> split(const simple_string& separator) const
	{
		simple_vector<simple_string> res;

		if (size <= separator.size)
			return {};

		size_t front = 0;
		size_t pos = 0;

		while (pos < (size - separator.size) )
		{
			simple_string act_str{ buffer + pos, separator.size };
			if (act_str == separator)
			{
				simple_string asd{ buffer + front, pos - front };
				res.append({ buffer + front, pos - front });
				pos += separator.size - 1;
				front = pos + 1;
			}
			pos++;
		}

		if (pos != size)
		{
			res.append({ buffer + front, size - front });
		}

		return res;
	}
};

class node
{
public:
	node()
	{}

	static node end;

	simple_string name;
	simple_vector<node> childs;

	simple_string value; 

	node& get_element_by_name(const simple_string& element_name)
	{
		// TODO could use map or implement own hashtable search
		for (size_t i = 0; i < childs.get_size(); ++i)
		{
			if (childs[i].name == element_name)
				return childs[i];
		}

		return end;
	}
};

node node::end;

class element_node : public node
{
public:
	element_node(const simple_string& _name, const simple_string& _value)
		: node()
	{
		name = _name;
		value = _value;
	}
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

	void clear()
	{
		childs.clear();
	}

	void print()
	{
		std::cout << std::endl << "ROOT" << std::endl;
		for (size_t g = 0; g < childs.get_size(); ++g)
		{
			group_node& group = (group_node&)childs[g];
			for (size_t e = 0; e < group.childs.get_size(); ++e)
			{
				std::cout << "  " << group.name.c_str() << " / " << group.childs[e].name.c_str() << " = " << group.childs[e].value.c_str() << std::endl;
			}
		}
	}
};



class file_base
{
public:
	file_base(const simple_string& file_name)
	{
		name = file_name;
	}

	virtual ~file_base()
	{}

	virtual simple_string read() = 0;
	virtual bool wait_for_change() = 0;

protected:
	simple_string name;
};

class win32_file : public file_base
{
public:
	win32_file(const simple_string& name) : file_base(name)
	{}

	~win32_file()
	{}

	simple_string read() override
	{
		const size_t buf_size = 5;
		char buf[buf_size];
		simple_string res;

		HANDLE hFile = CreateFileA(name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return {};

		while (true)
		{
			DWORD cnt = 0;
			if (ReadFile(hFile, buf, buf_size, &cnt, NULL) == FALSE)
			{
				CloseHandle(hFile);
				return {};
			}

			if (cnt < 0)
			{
				CloseHandle(hFile);
				return {};
			}

			res.append(buf, cnt);

			if (cnt < buf_size)
			{
				CloseHandle(hFile);
				return res;
			}
		}
		CloseHandle(hFile);
		return {};
	}

	bool wait_for_change() override 
	{
		char full_path[MAX_PATH];

		if (GetCurrentDirectoryA(MAX_PATH, full_path) == 0)
			return false;

		HANDLE h = FindFirstChangeNotificationA(full_path, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (h == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			return false;
		}

		if (WaitForSingleObjectEx(h, 1000, FALSE) == WAIT_OBJECT_0)
		{
			CloseHandle(h);
			return true;
		}
		return false;
	}
};

class ini_file_data
{
public:
	ini_file_data(const simple_string& data)
	{
		root.clear();

		simple_vector<simple_string> lines = data.split("\r\n");

		size_t linenum = 0;

		group_node* act_group_node = nullptr;

		while (linenum < lines.get_size())
		{
			simple_string act_line = lines[linenum];
			if (act_line.substring(0, 1) == "[")
			{
				if (act_group_node != nullptr)
				{
					root.append_child(*act_group_node);
					delete act_group_node;
				}

				act_group_node = new group_node{ act_line.substring(1, act_line.get_size() - 1) };
			}
			else
			{
				if (act_group_node != nullptr)
				{
					simple_vector<simple_string> key_value_pair = act_line.split(" = ");
					act_group_node->append_child({ key_value_pair[0], key_value_pair[1] });
				}
			}
			linenum++;
		}

		if (act_group_node)
		{
			root.append_child(*act_group_node);
			delete act_group_node;
		}
	}

	root_node root;
};

int main()
{
	win32_file file("a.ini");
	simple_string ini_data = file.read();

	ini_file_data ifd(ini_data);
	ifd.root.print();

	while (true)
	{
		bool res = file.wait_for_change();
		if (res)
		{
			simple_string new_ini_data = file.read();
			if (new_ini_data != ini_data)
			{
				//change
				ini_data = new_ini_data;
				ini_file_data ifd2(ini_data);
				ifd2.root.print();
			}
		}
	}

	const char *d = ini_data.c_str();

	return 0;
}
