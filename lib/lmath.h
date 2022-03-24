 #ifndef LMATH_H
 #define LMATH_H

 #include "math.h"
 #include "qlist.h"
 #include "qvector.h"


//////////////////////
class LMath
{
public:

    //random int form a to b, a <= b
    static int rndInt(uint a, uint b);
    static double rnd();
    static bool factorOk(double k);
    static double min(const QVector<double> &v);
    static double max(const QVector<double> &v);
    static double pi() {return 3.14159265;}
    static int sign(double a) {return ((a < 0) ? -1 : 1);}
    static double sqrt(const double&, const double &exact = 0.001);

};





 #endif
