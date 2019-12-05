// test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>

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
		}
		return true;
	}

	bool operator!=(const simple_vector<T>& other)
	{
		return !(operator==(other));
	}

	simple_vector<T> substring(size_t from, size_t to = MAX_SIZE_T)
	{
		if (to > size)
			to = size;

		return { buffer + from, from - size };
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
};

class node
{
public:
	node()
	{}

	static node end;

	simple_string name;
	simple_vector<node> childs;

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

class config
{
public:
	config()
	{}

	void load(const simple_string& file_name)
	{
		
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

int main()
{
	root_node root{};

	group_node group1("GROUP1");
	group_node group2("GROUP2");

	group1.append_child({ "element1" });
	group1.append_child({ "element2" });

	group2.append_child({ "element3" });
	group2.append_child({ "element4" });

	root.append_child(group1);
	root.append_child(group2);

	win32_file file("a.ini");
	simple_string ini_data = file.read();

	while (true)
	{
		bool res = file.wait_for_change();
		if (res)
		{
			simple_string new_ini_data = file.read();
			if (new_ini_data != ini_data)
			{
				//change

			}
		}
	}

	const char *d = ini_data.c_str();

	return 0;
}
