#include "data_utility.h"
#include <fstream>


 size_t DataUtility::findNextTo2Exp(size_t x)
{
    size_t acc = 1;
    while(acc < x)
        /* Jednoduché násobení dvojkou přes bitový posun */
        acc <<= 1;
    return acc;
}

 complex DataUtility::fromCharsToComplex(char *buffer, size_t length)
 {
     switch(length)
     {
     case 1:
         /* Nejdřív převede Bajt ze vstupu na unsigned char, poté na double a pak z nej udělá komplexní číslo */
         return complex((double)static_cast<unsigned char>(buffer[0]));
     case 2:
     {
         /* Nejdřív převede dva Bajty ze vstupu na short a pak z nej udělá komplexní číslo */
         return complex(*reinterpret_cast<short*>(buffer));
     }
     default:
         std::cerr << "Spatna velikost dat v fromCharsToComplex." << std::endl;
         return complex(0);
     }
 }

 std::vector<char> DataUtility::fromComplexToChars(complex x, size_t length)
 {
     std::vector<char> ret;
     short tmp2;
     switch(length)
     {
     case 1:
         /* Ořízne realnou část komplexního čísla a vlozí ho do výstupního vektoru */
         ret.push_back(static_cast<char>(x.re()  > 255 ? 255 : (x.re()  < 0 ? 0 : x.re())));
         return ret;
     case 2:
         /* Ořízne realnou část komplexního čísla a vlozí ho do výstupního vektoru */
         tmp2 = static_cast<short>(x.re() > 32767 ? 32767 : (x.re() < -32768 ? -32768 : x.re()));
         ret.push_back(char(tmp2 & 0xff));
         ret.push_back(char((tmp2 >> 8) & 0xff));
         return ret;
     default:
         std::cerr << "Spatna velikost dat v fromCharsToComplex." << std::endl;
         return std::vector<char>();
     }

 }

 void DataUtility::scaleComplex(complex &first, size_t size_of_sample, bool inverse)
 {
    switch(size_of_sample)
    {
        case 1:
        {
            if(!inverse) //from 0-255 to -1.0 - +1.0
                /* Přemění unsigned char na signed char a pak ho normalizuje na rozsah [-1,+1] */
                first = (first - complex(128)) / 128;
            else
                /* Denormalizuje na rozsah signed charu a posune na rozsah na unsigned char */
                first = first * 128 + complex(128);
            break;
        }
        case 2:
        {
            if(!inverse)
                /* Normalizuje na rozsah [-1,+1] */
                first /= 32768;
            else
                /* Denormalizuje na rozsah signed shortu */
                first *= 32768;
            break;
        }
    }
 }

 std::vector<double> DataUtility::loadPreset(const char *filename)
 {
     std::ifstream in;
     in.open(filename, std::ios_base::in | std::ios_base::binary);
     std::vector<double> preset;
     double buffer = 0;
     while(!in.eof())
     {
        in >> buffer;
        preset.push_back(buffer);
     }

     in.close();
     return preset;
 }
