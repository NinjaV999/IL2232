//
// Created by yz on 2/23/23.
//

#ifndef HELLO_VCNI_MMP_H
#define HELLO_VCNI_MMP_H
#include <stdlib.h>

#include <iostream>


class VCNI_MMP
{
public:
    VCNI_MMP( );
    int VCNIMMP_alpha1, VCNIMMP_beta1, VCNIMMP_base1, VCNIMMP_base2;
    int VCNIMMP_state1, VCNIMMP_state2; // off -> 0; on -> 1;
    int actionAlpha2;
    bool VCNIMMPOnOrOff(int alpha, int beta, int base, int &state);
    bool VCNIMMPRequest(int t_actionAlpha2);
    virtual ~VCNI_MMP();


};


#endif //HELLO_VCNI_MMP_H
