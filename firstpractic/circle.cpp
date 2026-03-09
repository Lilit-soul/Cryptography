#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

int main(){

    const double PI = acos(-1.0); // число пи (аркосинус -1)
    double radius;
    double corner;
    int rounding;

    cout << "Введите радиус: ";
    cin >> radius;
    cout << "Введите угол: ";
    cin >> corner;
    cout << "Введите число для округления: ";
    cin >> rounding;

    double circuit = 2 * PI * radius;// Длина окружности
    double area = PI * radius * radius;// Площадь круга
    double sector = (corner / 360) * area;// Площадь кругового сектора

    cout << "Длина окружности: " << setprecision(rounding) << circuit << "\n";
    cout << "Площадь круга: "<< setprecision(rounding)<< area << "\n";
    cout << "Площадь кругового сектора: " <<setprecision(rounding)<< sector << "\n";
    
}
