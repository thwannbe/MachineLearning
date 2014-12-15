#include "concept.h"
#include <iostream>

using namespace std;

main(void) {
    ConceptLearning* cl = new ConceptLearning;
    cl->Initialize(6);
    bool i[] = { true, true, false, true, true, false, true };
    cl->UpdateVersionSpace(i);
    
    i[2] = true;
    cl->UpdateVersionSpace(i);
    
    i[0] = false; i[1] = false; i[5] = true; i[6] = false;
    cl->UpdateVersionSpace(i);

    i[0] = true; i[1] = true; i[4] = false; i[6] = true;
    cl->UpdateVersionSpace(i);

    bool ui[] = { true, true, false, true, false, true };
    Result ret = cl->CoverCheck(ui);
    char RETURN[] = { 'F', 'T', '?' };
    cout << RETURN[ret] << endl;

    ui[0] = false; ui[1] = false; ui[3] = false; ui[4] = true; ui[5] = false;
    ret = cl->CoverCheck(ui);
    cout << RETURN[ret] << endl;

    ui[0] = true; ui[1] = true;
    ret = cl->CoverCheck(ui);
    cout << RETURN[ret] << endl;

    ui[1] = false; ui[3] = true;
    ret = cl->CoverCheck(ui);
    cout << RETURN[ret] << endl;

    return 0;
}
