#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <stdio.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "Python.h"

using namespace std::chrono_literals;

class CPyInstance
{
public:
	CPyInstance()
	{
		Py_Initialize();
	}

	~CPyInstance()
	{
		Py_Finalize();
	}
};


class CPyObject
{
private:
	PyObject *p;
public:
	CPyObject() : p(NULL)
	{}

	CPyObject(PyObject* _p) : p(_p)
	{}

	
	~CPyObject()
	{
		Release();
	}

	PyObject* getObject()
	{
		return p;
	}

	PyObject* setObject(PyObject* _p)
	{
		return (p=_p);
	}

	PyObject* AddRef()
	{
		if(p)
		{
			Py_INCREF(p);
		}
		return p;
	}

	void Release()
	{
		if(p)
		{
			Py_DECREF(p);
		}

		p= NULL;
	}

	PyObject* operator ->()
	{
		return p;
	}

	bool is()
	{
		return p ? true : false;
	}

	operator PyObject*()
	{
		return p;
	}

	PyObject* operator = (PyObject* pp)
	{
		p = pp;
		return p;
	}

	operator bool()
	{
		return p ? true : false;
	}
};

/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
  public:
    MinimalPublisher()
    : Node("minimal_publisher"), count_(0)
    {
      publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
      timer_ = this->create_wall_timer(
      500ms, std::bind(&MinimalPublisher::timer_callback, this));
    }

  private:
    void timer_callback()
    {
      auto message = std_msgs::msg::String();
      message.data = "Hello, world! " + std::to_string(count_++);
      RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
      publisher_->publish(message);
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t count_;
};

int main(int argc, char * argv[])
{

  CPyInstance inst;
  CPyObject pName = PyUnicode_FromString("test_pkg.helloworld");

  CPyObject pModule = PyImport_Import(pName);

  if(pModule)
	{
		CPyObject pFunc = PyObject_GetAttrString(pModule, (char*)"printwrld");
		if(pFunc && PyCallable_Check(pFunc))
		{
			CPyObject pArgs = PyTuple_Pack(1, PyUnicode_FromString((char*)"Sent and got this string through Python!"));
			CPyObject pValue = PyObject_CallObject(pFunc, pArgs);
			auto result = _PyUnicode_AsString(pValue);
			std::cout << result << std::endl;
			//printf("C: printwrld() = %ld\n", PyLong_AsLong(pValue));
		}
		else
		{
			printf("ERROR: function printwrld()\n");
		}

	}
	else
	{
		printf("ERROR: Module not imported\n");
	}

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}