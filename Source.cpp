//1. Different ways of creating an std::unique_ptr.
//2. How to access the value of a pointer using smart pointer?
//3. Different ways of passing an std::unique_ptr as parameter to a function
//4. How to return a smart pointer from a function?
//5. Usage of std::unique_ptr class member functions.

// First we need to include the header file <memory>
// This header file contains all the necessary implementations of all smart pointers
#include <memory>

// For demonstration puropse lets us use a Class named Tree.
#include "Tree.h"

void PassUniquePtrObjectByValue( std::unique_ptr<Tree> upTree_i ){
    std::cout << __FUNCTION__;
    std::cout << "Name : "<< upTree_i->m_strName << "Age : " << upTree_i->m_nAge << "\n";
}

void PassUniquePtrObjectByReference( std::unique_ptr<Tree>& upTree_i ){
    std::cout << __FUNCTION__;
    std::cout << "Name : "<< upTree_i->m_strName << "Age : " << upTree_i->m_nAge << "\n";        
}

void PassUniquePtrObjectByConstReference( const std::unique_ptr<Tree>& upTree_i ){
    std::cout << __FUNCTION__;
    std::cout << "Name : "<< upTree_i->m_strName << "Age : " << upTree_i->m_nAge << "\n";
}

void PassRawPointerAsReference( Tree& tree_i ) {
    std::cout << __FUNCTION__;
    std::cout << "Name : "<< tree_i.m_strName << "Age : " << tree_i.m_nAge << "\n";
}

void PassRawPointer( Tree* pTree_i ){
    std::cout << __FUNCTION__;
    std::cout << "Name : "<< pTree_i->m_strName << "Age : " << pTree_i->m_nAge << "\n";
}


std::unique_ptr<Tree> UniquePtrAsReturn(){
    auto LocalUniquePtrObject = std::make_unique<Tree>( "CoconutTree", 25 );
    return LocalUniquePtrObject;
}
// Here while returing, the local unique_ptr smart pointer stack object is moved as r-value reference &&.
// r-value reference is introduced in C++ 11.
// r-value references extend the lifespan of the object they are initialized with to the lifespan of the r-value reference

int main()
{
    // For creating a smart pointer one should
    // Declare the unique_ptr as an automatic (local) variable.
    // I.e., do not use the new or malloc expression on the smart pointer itself.

    // smart pointers are all template classes.
    // In the type parameter, specify the pointed-to type, of the encapsulated pointer.

    // Attach a raw pointer to smart pointer object using the unique_ptr constructor. 

    // Smart pointers can be created in 3 ways.
    // 1. From an already existing raw pointer
    {
        Tree *pTree1 = new Tree( "MangoTree1", 10 ) ;
        std::unique_ptr<Tree>Tree1( pTree1 );
        // or
        Tree *pTree2 = new Tree( "MangoTree2", 6 ) ;
        std::unique_ptr<Tree>Tree2 = std::unique_ptr<Tree>( pTree2 );
        // or
        Tree *pTree3 = new Tree( "MangoTree3", 15 ) ;
        auto Tree3 = std::unique_ptr<Tree>( pTree3 );

        // 2. Directly while creating the smart pointer object
        {
            auto tree = std::unique_ptr<Tree>( new Tree( { "AppleTree", 15 } ));
        }

        // 3. Using a dedicated function std::make_unique introduced in C++14
        {
            auto tree = std::make_unique<Tree>( "OrangeTree", 8 );
        }

        // According to C++ core guidelines, one should always prefer creating an
        // std::unique_ptr using the std::make_unique function.
        // std::make_unique have 3 adavantages
        // 1. It avoids the need for keyword 'new'. developer can completly avoid the usage
        // of new and delete keyword. If we ourself is not using new keyword, then
        // we don't need to worry about calling delete also. Here the new and delete shall be
        // Managed automatically by the smart pointer and its utility functions.
        // make_unique allows users to "do coding with out performing  manual memory allocations/deallocations using new/delete keywords".

        // 2. std::make_unique is carefully implemented for exception safety and is
        // recommended over directly calling unique_ptr constructors.

        // 3. with std::unique_ptr does not require redundant type usage,
        // the code is short. I.e., the Template class type is mentioned only once
        // Here Tree is specified 2 times
        {
            auto tree1 = std::unique_ptr<Tree>( new Tree( { "OrangeTree", 15 } ));
            // Here Tree is specified only once.
            auto tree2 = std::make_unique<Tree>( "OrangeTree", 8 );
        }

        // But std::make_unique method may not be suitable in cases like
        // 1. If a custom deleter is to be specified
        // 2. if adopting an existing raw pointer .

        // These are the different ways one can create a fresh smart pointer object.
    }

    // An existing unique_ptr smart pointer can transefer its ownership to a new unique_ptr object.
    {
        auto tree = std::make_unique<Tree>( "AppleTree", 9 );
        {
            auto SameTree = std::move( tree); 
            // Here move constructor is invoked as the unique_ptr object is being created and the ownerhsip is moved to new object
        }
        {
            auto SameTree = std::make_unique<Tree>();
            SameTree  = std::move( tree ); 
        }
        // Here move assignement operator is invoked as the unique_ptr object is already existing  and the ownerhsip is moved to new object
        // The move constructor and move assignemnt operators are introduced in standard library as part of C++11.

        // Once the ownership is transferred. the old object i.e., 'AppleTree' object becomes empty.
        // The Apple Tree pointer is now assoicated with the new unique_ptr object 'SameAppleTree'
    }
    // -------------------------------------------------------------
    // Now let us see How to access the value of a pointer using smart pointer?
    // It is very simple.
    // in all the smart pointer classes including unique_ptr, it has overlaoded the operator *, ->
    // so one can access the pointer objects members (varaibles/functions) by using the * and -> operators of smart pointers
    {
        auto tree = std::make_unique<Tree>( "AppleTree", 9 );
        tree->m_nAge += 3; // Increament the age of apple tree by 3. Now age becomes 12
        int nAge = 0;
        nAge = tree->m_nAge;
    }


    // -----------------------------------------------------------------
    // Now lets see how to pass the unique_ptr as parameters of functions.
    // While coding, one might some times need to pass the unique_ptr to function as parameters
    // one can pass the paramters in different ways.
    // it can be pass by value or pass by reference or as a const reference paramter
    // One can also decide to pass the undelying pointer directly as the function parameter.
    {
        void PassUniquePtrObjectByValue( std::unique_ptr<Tree> upTree_i );
        void PassUniquePtrObjectByReference( std::unique_ptr<Tree>& upTree_i );
        void PassUniquePtrObjectByConstReference( const std::unique_ptr<Tree>& upTree_i );
        void PassRawPointerAsReference( Tree& tree_i ); // it can also be const reference paramter
        void PassRawPointer( Tree* pTree_i ); // it can also be const pointer paramter

        {
            auto tree = std::make_unique<Tree>( "AppleTree", 5 );
            // PassUniquePtrObjectByValue( AppleTree ); This casues compilation error
            // because, unique_ptr cannot be copied. When a parameter is passed as by value to a function
            // temporary objects are created by invoking the Copy Constructor. 
            // As per unique_ptr concept, the copy is prohibted. I.e., the copy constructor is marked as delete.
            // Thus providing Exclusing ownership. I.e., the object is not copyable means it is not sharing 
            // the undelying pointer with any other unique_ptr's.

            PassUniquePtrObjectByValue( std::move( tree ));
            // For this we need to move ownership of the underlying pointer to the function by using std::move function.
            // After this, the AppleTree unique_ptr object becomes empty. It is no more have onwership of the Tree pointer.
        
            tree = std::make_unique<Tree>( "AppleTree", 5 );
            PassUniquePtrObjectByReference( tree );
            // This is just as how we pass a paramter as reference. Noting special with this case.
            // Here the address of AppleTree obejct is passed. 
            // In this cases the AppleTree object is valid even after the function call.
            // the only thing the function might doing is to access/Update the member varaible of the Tree pointer.

            PassUniquePtrObjectByConstReference( tree );
            // It is same as that of PassUniquePtrObjectByReference, but in this cases,
            // the function cannot modifiy the smart pointer and the Tree Pointer object.
            // No other differences.

            PassRawPointerAsReference( *tree );
            // By using the unique_ptr::* operator we get the reference address to the underlying Tree pointer;

            PassRawPointer( tree.get() );
            // By using the unique_ptr::get() function we can obtain the pointer address to the underlying Tree pointer;
        }
        
        // So how we decide which one is to be used.
        // If we need to tansfer the ownership for the smart pointer, then we have to perfer PassByValue.
        // e.g., say we are passing the unique_ptr object as a parameter to a thread function.
        // In most cases after passing the paramter top the thread function there is no use of this unique_ptr in the main thread. 
        // in this case use PassByValue.

        // If we just calling a blocking function call which does not going to take the ownership or change the smart pointer
        // Then then we can use any of the other functions. I.e pass the parameter either as unique_ptr reference or
        // as row pointer reference/pointer itself.
        // According to C+ core guideline, it is preffered to pass the parameter as raw pointer of the undelying type itslef.
        // No need to pass as unique_ptr

    }
    // --------------------------------------------------------------------
    // How to return a smart pointer from a function?
    // To return a unique_ptr smart pointer, one only need to return the corresponding smart pointer by value.
    {
        std::unique_ptr<Tree> UniquePtrAsReturn();
        auto upAppleTree = UniquePtrAsReturn();;
    }

    // --------------------------------------------------------------------
    // Usage of std::unique_ptr class member functions.
    // std::unique_ptr class has some utility functions for manupulating the undelying objects ownership.
    {
        auto AppleTree = std::make_unique<Tree>( "AppleTree", 5 );
        Tree *pTree  = AppleTree.get();                      // used to get the undelying raw Tree pointer.
        AppleTree.reset( new Tree( "AnotherAppleTree", 1 )); // The old Tree object is deleted and take the ownership of new Tree pointer "AnotherAppleTree"
        Tree *pTree1 = AppleTree.release();                  // Release the ownership. After that the unique_ptr object is Empty.
    }
    return 0;
}