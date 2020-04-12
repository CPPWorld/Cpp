#pragma once
#include<iostream>
#include <string>
#include <windows.h>

class Tree {
public: // Member function section
    // Default Constructor of class Tree
    Tree() {
        std::cout << "[Tree default constructor]\n";
    };

    // Initialization Constructor used for member variable initialization.
    Tree( const std::string strName_i, int nAge_i ) :
        m_strName{ strName_i }, m_nAge{ nAge_i } {
        std::cout << "[Tree intitialization constructor]";
        std::cout << "Name : "<< m_strName << "Age : " << m_nAge << "\n";
    }

    // Destructor of class Tree
    ~Tree( ) {
        std::cout << "[Tree destructor]\n";
    }

public: // Member varaibles section
    std::string m_strName;
    int m_nAge    = 0;
};