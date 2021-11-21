
// #define PERFORM_TESTS

#ifndef PERFORM_TESTS

#include "tile/Application.inl"

int main()
{   
    Application app;
    app.Start();
}

#else

#include "tests/load_model.inl"

int main()
{   
    load_model_test_main();
}

#endif