//
// Created by yz on 2/23/23.
//

#include "VCNI_MMP.h"

VCNI_MMP::VCNI_MMP() {//似乎没用
    //srand(10);
    VCNIMMP_alpha1 = 25;
    VCNIMMP_beta1 = 0;
    VCNIMMP_base1 = VCNIMMP_alpha1 + VCNIMMP_beta1;

    VCNIMMP_base2 = 100;
    actionAlpha2 = VCNIMMP_base2;// default: all state1, all nodes open
}

bool VCNI_MMP::VCNIMMPOnOrOff(int VCNIMMP_alpha, int VCNIMMP_beta, int VCNIMMP_base, int &VCNIMMP_state) {
    if (VCNIMMP_state == 0) { // off
        if (rand() % VCNIMMP_base < VCNIMMP_alpha) {//  STATE1 7/25
            VCNIMMP_state = 1;
            return true;
        } else {
            return false;
        }
    } else {  // on
        if (rand() % VCNIMMP_base < VCNIMMP_beta) {
            VCNIMMP_state = 0;
            return false;
        } else
            return true;
    }
}

  // previous func only delay and not control the rate
bool VCNI_MMP::VCNIMMPRequest(int t_actionAlpha2) {
    if (true) {
        //if (OnOrOff(alpha1, beta1, base1, state1)) {// first layer on
        if (VCNIMMPOnOrOff(t_actionAlpha2, (VCNIMMP_base2 - t_actionAlpha2), VCNIMMP_base2, VCNIMMP_state2)) {//first layer on，check second layer
            if (t_actionAlpha2 != 100) {
                //std::cout << " " << actionAlpha2 << " actionAlpha2 in VCNIMMP " << t_actionAlpha2 << "\n";//yz
            }
            return true;
        }
        else{
        //std::cout<<actionAlpha2 <<" "<<t_actionAlpha2<<" actionAlpha2 in VCNIMMP \n";//second layer fail
            if (t_actionAlpha2 != 100) {
                //std::cout << " actionAlpha2Used " << actionAlpha2 << " actionAlpha2InVCNIMMP " << t_actionAlpha2 << "\n";//yz
            }
        return false;//fail at second layer
        }
    } else {//first layer off, return final stat off/false
        VCNIMMP_state1 = 0;
        return false;
    }

}

VCNI_MMP::~VCNI_MMP() {

}