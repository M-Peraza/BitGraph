#include <algorithm>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>


/* EJEMPLO de MOVE const
class MyClass{

private:

    std::string data;
    double value;

public:

    explicit MyClass(const std::string& string_inicial,const double& val): data(string_inicial), value(val) {}

    MyClass& operator=(MyClass&& other) noexcept {
        data = std::move(other.data);
        value = other.value;
        return *this;
    }

};

int main(){

    MyClass obj1("Set necesita animales", 5.0);
    MyClass obj2("Hola", 1.0);
    obj2 = std::move(obj1);
    std::cout << "HHHH";
    return 0;

}
*/


/*
struct Multiply
{
    double factor;
    Multiply(double f): factor(f){};  
    double operator()(double number){
        return number * factor;
    }
    
};

int main(){

std::vector<double> celsius{1,34,12,7,-3};
std::vector<double> fahrenheit(celsius.size());
    
std::transform(celsius.begin(), celsius.end(), fahrenheit.begin(), Multiply(1.2));

std::cout<<"Besiqueñes";

}
*/

//////////////////////////////////////////////////////////SEP////////////////////////////////////

/*class MyClass {
private:
    std::string data;
    int value;

public:
    // Copy constructor (might throw due to string allocation)
    MyClass(const MyClass& other) : data(other.data), value(other.value) {}
    
    // Move constructor marked noexcept
    MyClass(MyClass&& other) noexcept 
        : data(std::move(other.data)), value(other.value) {}
    
    // Move assignment marked noexcept  
    MyClass& operator=(MyClass&& other) noexcept {
        data = std::move(other.data);
        value = other.value;
        return *this;
    }
};*/
