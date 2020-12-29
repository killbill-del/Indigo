#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"
#include "molecule/structure_checker2.h"

const char* FAILED_MOLECULE_STRING = "The molecule\n"
                                     "  Ketcher 12222012222D 1   1.00000     0.00000     0\n"
                                     "\n"
                                     "  6  6  0     0  0            999 V2000\n"
                                     "    6.2250   -5.4250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "    7.0910   -5.9250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "    7.0910   -6.9250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "    6.2250   -7.4250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "    5.3590   -6.9250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "    5.3590   -5.9250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                     "  1  2  1  0     0  0\n"
                                     "  2  3  2  0     0  0\n"
                                     "  3  4  1  0     0  0\n"
                                     "  4  5  2  0     0  0\n"
                                     "  5  6  1  0     0  0\n"
                                     "  6  1  2  0     0  0\n"
                                     "M  RBC  1   4   2\n"
                                     "M  END\n";

const char* GOOD_MOLECULE_STRING = " The molecule\n"
                                   "  Ketcher 08191612012D 1   1.00000     0.00000     0\n"
                                   "\n"
                                   " 36 35  0     0  0            999 V2000\n"
                                   "    7.8000   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "    8.6660   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "    9.5321   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   10.3981   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   11.2641   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   12.1301   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   12.9962   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   13.8622   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   14.7282   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   15.5942   -7.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   16.4603   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   17.3264   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   18.1924   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   19.0584   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   19.9245   -8.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   20.7905   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "    8.6660   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "    9.5321   -6.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   10.3981   -8.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   11.2641   -6.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   12.9962   -9.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   13.8622   -6.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   14.7282   -9.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   15.5942   -6.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   17.3264   -9.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   18.1924   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   19.0584   -9.5250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   21.4976   -9.2321    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   22.4635   -8.9733    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   23.1706   -9.6804    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   19.9245   -7.0250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   22.7223   -8.0074    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   21.2388  -10.1980    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   21.2905   -7.6590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   12.6301   -6.6590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "   16.9603   -7.1590    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                   "  1  2  1  0     0  0\n"
                                   "  2  3  1  0     0  0\n"
                                   "  3  4  1  0     0  0\n"
                                   "  4  5  1  0     0  0\n"
                                   "  5  6  1  0     0  0\n"
                                   "  6  7  1  0     0  0\n"
                                   "  7  8  1  0     0  0\n"
                                   "  8  9  1  0     0  0\n"
                                   "  9 10  1  0     0  0\n"
                                   " 10 11  1  0     0  0\n"
                                   " 11 12  1  0     0  0\n"
                                   " 12 13  1  0     0  0\n"
                                   " 13 14  1  0     0  0\n"
                                   " 14 15  1  0     0  0\n"
                                   " 15 16  1  0     0  0\n"
                                   "  2 17  1  1     0  0\n"
                                   "  3 18  1  1     0  0\n"
                                   "  4 19  1  1     0  0\n"
                                   "  5 20  1  1     0  0\n"
                                   "  7 21  1  1     0  0\n"
                                   "  8 22  1  1     0  0\n"
                                   "  9 23  1  1     0  0\n"
                                   " 10 24  1  1     0  0\n"
                                   " 12 25  1  1     0  0\n"
                                   " 13 26  1  1     0  0\n"
                                   " 14 27  1  1     0  0\n"
                                   " 16 28  1  0     0  0\n"
                                   " 28 29  1  0     0  0\n"
                                   " 29 30  1  0     0  0\n"
                                   " 15 31  1  1     0  0\n"
                                   " 29 32  1  1     0  0\n"
                                   " 28 33  1  1     0  0\n"
                                   " 16 34  1  1     0  0\n"
                                   "  6 35  1  1     0  0\n"
                                   " 11 36  1  1     0  0\n"
                                   "M END\n";

int test(int negative)
{
    int qm = indigoLoadQueryMoleculeFromString(negative ? FAILED_MOLECULE_STRING : GOOD_MOLECULE_STRING);
    if (qm < 0)
    {
        fprintf(stderr, "Error: can't parse input; %d is returned by indigoLoadQueryMoleculeFromString\n", qm);
    }
    const char* result = indigoCheck(qm, 0);
    printf(!result ? "NULL" : result);
    indigoFree(qm);
    return 0;
}

int test2(int negative)
{
    indigo::StructureChecker2 checker;
    std::string result = checker.check(negative ? FAILED_MOLECULE_STRING : GOOD_MOLECULE_STRING, "query,stereo").toJson();
    printf(result.c_str());
    return 0;
}

void onError(const char* message, void* context)
{
//    fprintf(stderr, "Error: %s\n", message);
//    exit(-1);
}

int main(void)
{
    indigoSetErrorHandler(onError, 0);
    printf("Issue #269 input -- negative test\n");
    test(1);
    printf("\n\nTest2 -- negative test\n");
    test2(1);
    // printf("\n\nPositive test\n");
    // test(0);
    return 0;
}
