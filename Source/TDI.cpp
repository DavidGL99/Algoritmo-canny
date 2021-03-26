#include <cmath>
#include <fcntl.h>
#include "iostream"
#include <time.h>
#include <C_General.hpp>
#include <C_File.hpp>
#include <C_Arguments.hpp>
#include <C_Matrix.hpp>
#include <C_Image.hpp>
#include <C_Trace.hpp>


using namespace std;

int Test(int argc, char** argv);

void desenfoque_gaussiano_5x5(C_Image&, C_Image&);
void sobel_operator(C_Image&, C_Image&, C_Image&);
void definir_bordes(C_Image&, C_Image&, C_Image&);
void hysteriesis(C_Image&, C_Image&, C_Image&, int, int);

int main(int argc, char** argv) {

		
	cout << "BIENVENIDO" << endl;
	cout << "Este programa cargara una imagen y le aplicara el algoritmo de canny." << endl;
	cout << "Las imagenes a con las que se quiera trabajar deberan estar ubicadas en la carpeta RUN de este proyecto, y debe tratarse de un archivo .bmp sin comprimir, en escala de grises" <<
		" y con 8 bits de profundidad de color, muchas gracias." << endl;
	cout << "Por favor, que imagen quieres procesar? (E.J  xxx.bmp) : ";
	
	C_Image::IndexT row, col;
	C_Image image;
	clock_t initTime;
	
	initTime = clock();
	
	string imagen;
	
	cin >> imagen;
	
	cout<<("Leyendo la imagen...");
	
	image.ReadBMP(imagen.c_str());
	
	if (image.Fail()) {
		cout << "Lo sentimos, no hemos encontrado la imagen. "<< endl << "Por favor, asegurese de que se ha escrito correctamente, tiene el formato especificado, o se encuentra corerctamente ubicada en la carpeta RUN" << endl;
			return -1;
	}
	
	C_Image convolution_image(image.FirstRow(), image.LastRow(), image.FirstCol(), image.LastCol(), 0);
	C_Image _sobel(1, image.LastRow(), 1, image.LastCol(), 0);
	C_Image direction_image(image.FirstRow(), image.LastRow(), image.FirstCol(), image.LastCol(), 0);
	C_Image imagen_supress(image.FirstRow(), image.LastRow(), image.FirstCol(), image.LastCol(), 0);
	C_Image final_image(image.FirstRow(), image.LastRow(), image.FirstCol(), image.LastCol(), 0);
	
		
	cout << "Aplicando filtro gaussiano..." << endl;
	desenfoque_gaussiano_5x5(image, convolution_image);
	
	cout << "Correcto!" << endl;
	cout << "Aplicando operador de Sobel..." << endl;
	sobel_operator(convolution_image, _sobel, direction_image);
	cout << "Correcto!" << endl;
	
		
	cout << "Suprimiendo los pixeles no maximos..." << endl;
	
	definir_bordes(_sobel, imagen_supress, direction_image);
	cout << "Correcto!" << endl;
	
		
	int max, min;
	cout << "Por favor inserte un valor maximo de thresholding : ";
	cin >> max;
	cout << "Por favor inserte un valor minimo de thresholding : ";
	cin >> min;
	
	cout << "Aplicando histeresis..." << endl;
	hysteriesis(imagen_supress, _sobel, final_image, max, min);
	cout << "Correcto!" << endl;
	
	
	cout << "La imagen se ha guardado en la carpeta run con el nombre de :" << imagen.substr(0, imagen.size() - 4) << "_canny.bmp" << endl;
	
	string salida = imagen.substr(0, imagen.size() - 4) += "_canny.bmp";
	
	final_image.WriteBMP(salida.c_str());
	return 0;
}

//Hacemos uso de un kernel gaussiano, y se lo aplicamos a la imagen dada por parametro
void desenfoque_gaussiano_5x5(C_Image & image, C_Image & out) {
	
	C_Matrix kernel(1, 5, 1, 5, 1);
	kernel(1, 1) = 1;	kernel(1, 2) = 4;	kernel(1, 3) = 7; kernel(1, 4) = 4; kernel(1, 5) = 1;
	kernel(2, 1) = 4;	kernel(2, 2) = 16;	kernel(2, 3) = 26; kernel(2, 4) = 16; kernel(2, 5) = 4;
	kernel(3, 1) = 7;	kernel(3, 2) = 26;	kernel(3, 3) = 41; kernel(3, 4) = 26; kernel(3, 5) = 7;
	kernel(4, 1) = 4;	kernel(4, 2) = 16;	kernel(4, 3) = 26; kernel(4, 4) = 16; kernel(5, 5) = 4;
	kernel(5, 1) = 1;	kernel(5, 2) = 4;	kernel(5, 3) = 7; kernel(5, 4) = 4; kernel(5, 5) = 1;
	
	for (C_Image::IndexT fila = image.FirstRow() + 2; fila <= image.LastRow() - 2; fila++) {
		for (C_Image::IndexT col = image.FirstCol() + 2; col <= image.LastCol() - 2; col++) {
			
			int media = 0;
			C_Image::IndexT q = 1;
			
				for (int i = fila - 2; i <= fila + 2; i++) {
				C_Image::IndexT k = 1;
				for (int j = col - 2; j <= col + 2; j++) {
					media += (image(i, j) * kernel(q, k));
					k++;
					
				}
				 q++;
				
			}
			 out(fila, col) = media / 273;
			
				
		}
		
	}
	
}



void sobel_operator(C_Image & image, C_Image & image_out, C_Image & direction_image) {
	C_Matrix kernelX(1, 3, 1, 3, 0);
	kernelX(1, 1) = -1;	kernelX(1, 2) = 0;	kernelX(1, 3) = 1;
	kernelX(2, 1) = -2;	kernelX(2, 2) = 0;	kernelX(2, 3) = 2;
	kernelX(3, 1) = -1;	kernelX(3, 2) = 0;	kernelX(3, 3) = 1;
	
	C_Matrix kernelY(1, 3, 1, 3, 0);
	kernelY(1, 1) = -1;	kernelY(1, 2) = -2;	kernelY(1, 3) = -1;
	kernelY(2, 1) = 0;	kernelY(2, 2) = 0;	kernelY(2, 3) = 0;
	kernelY(3, 1) = 1;	kernelY(3, 2) = 2;	kernelY(3, 3) = 1;
	
	
	
	for (C_Image::IndexT fila = image.FirstRow() + 1; fila <= image.LastRow() - 1; fila++) {
		for (C_Image::IndexT col = image.FirstCol() + 1; col <= image.LastCol() - 1; col++) {
			float gy = 0;
			float gx = 0;
			float direction, estimed_direction = 0.0;
			C_Image::IndexT q = 1;
			
				for (C_Image::IndexT i = fila - 1; i <= fila + 1; i++) {
					C_Image::IndexT k = 1;
					for (C_Image::IndexT j = col - 1; j <= col + 1; j++) {
						gx += (image(i, j) * kernelX(q, k));
						gy += (image(i, j) * kernelY(q, k));
						k++;
					
					}
					q++;
				
				}
				int g = sqrt(pow(gx, 2.0) + pow(gy, 2.0));
				image_out(fila, col) = g > 255 ? 255 : g;
			
				direction = (atan2(gx, gy) / M_PI) * 180.0;
			
				if (((direction < 22.5) && (direction > -22.5)) || (direction > 157.5) && (direction < -157.5)) {
					estimed_direction = 0;
				
				}
				if (((direction > 22.5) && (direction < 67.5)) || ((direction < -112.5) && (direction > -157.5))) {
					estimed_direction = 45;
				
				}
				if (((direction > 67.5) && (direction < 112.5)) || ((direction < -67.5) && (direction > -112.5))) {
					estimed_direction = 90;
				
				}
				if (((direction > 112.5) && (direction < 157.5)) || ((direction < -22.5) && (direction > -67.5))) {
					estimed_direction = 135;
				
				}
			
			direction_image(fila, col) = estimed_direction;
			
		}
		
	}
	
}

void definir_bordes(C_Image& image, C_Image& image_out, C_Image& direction_image) {
	
		for (C_Image::IndexT fila = image.FirstRow() + 1; fila <= image.LastRow() - 1; fila++) {
			for (C_Image::IndexT col = image.FirstCol() + 1; col <= image.LastCol() - 1; col++) {
			
				
				switch (int(direction_image(fila, col))) {
				case 0:
					if ((image(fila - 1, col) > image(fila, col) || image(fila + 1, col) >= image(fila, col))) {
						image_out(fila, col) = 0;
						
					}
					else {
						image_out(fila, col) = 255;
						
					}
					break;
				case 45:
					if ((image(fila - 1, col - 1) > image(fila, col) || image(fila + 1, col + 1) >= image(fila, col))) {
						
							image_out(fila, col) = 0;
						
					}
					else {
						image_out(fila, col) = 255;
						
					}
					break;
				case 90:
					if ((image(fila, col - 1) > image(fila, col) || image(fila, col + 1) >= image(fila, col))) {
						image_out(fila, col) = 0;
						
							
					}
					else {
						image_out(fila, col) = 255;
						
					}
					break;
				case 135:
					if ((image(fila + 1, col - 1) > image(fila, col) || image(fila - 1, col + 1) >= image(fila, col))) {
						image_out(fila, col) = 0;
						
					}
					else {
						image_out(fila, col) = 255;
						
					}
					break;
					
				default:
					break;
					
				}
			
				
				
			}
		
		}
	
}


void hysteriesis(C_Image & image, C_Image & sobel, C_Image & image_out, int max, int min) {
	C_Image es_borde(image.FirstRow(), image.LastRow(), image.FirstCol(), image.LastCol(), 0);
	
		
		for (C_Image::IndexT fila = image.FirstRow() + 1; fila <= image.LastRow() - 1; fila++) {
			for (C_Image::IndexT col = image.FirstCol() + 1; col <= image.LastCol() - 1; col++) {
				if (sobel(fila, col) > max) {
					es_borde(fila, col) = 1;
				
				}
			
				if (sobel(fila, col) < min) {
					es_borde(fila, col) = -1;
				
				}
			
				if (sobel(fila, col) > min && sobel(fila, col) < max) {
					es_borde(fila, col) = 2;
				
				}
			
			}
		
		}
	
		for (C_Image::IndexT fila = image.FirstRow() + 1; fila <= image.LastRow() - 1; fila++) {
			for (C_Image::IndexT col = image.FirstCol() + 1; col <= image.LastCol() - 1; col++) {
			
				if (es_borde(fila, col) == 2) {
					if (es_borde(fila - 1, col - 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila - 1, col + 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila, col - 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila, col + 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila + 1, col - 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila + 1, col) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila + 1, col + 1) == 1) {
						es_borde(fila, col) = 1;
					
					}
					if (es_borde(fila - 1, col) == 1) {
						es_borde(fila, col) = 1;
					
					}
				
				}
			
				
				es_borde(fila, col) == 1 ? image_out(fila, col) = image(fila, col) : image_out(fila, col) = 0;
			
			}
		
		}
	
		
}
