#ifndef MYCLASS_HPP
#define MYCLASS_HPP

class MyClass {
public:
    MyClass(int initialValue); // Constructor
    void setValue(int newValue); // Setter
    int getValue(); // Getter
    void printValue(); // Method to print the value

private:
    int value;
};

#endif // MYCLASS_HPP
