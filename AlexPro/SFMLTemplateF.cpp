#define NOMINMAX
#include "Logica.h"
#include "SFML.h"

void static procesarCarpeta(const std::wstring& ruta) {
    ListaEnlazada listaHashes;
    wcout << L"Procesando la carpeta: " << ruta << L"...\n";
    listar_y_hashear(ruta, listaHashes);
    listaHashes.mostrar();
    listaHashes.mostrarDuplicados();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    Ventana ventana(procesarCarpeta);
    ventana.run();
    return 0;
}