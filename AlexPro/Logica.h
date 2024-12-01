#ifndef LOGICA_H
#define LOGICA_H

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <windows.h>
#include "Md5hash.h"
#include <map>
using namespace std;

inline wstring normalizarNombre(const wstring& nombre) {
    wstring nombreBase = nombre;
    size_t ultimoPunto = nombreBase.find_last_of(L'.');
    wstring extension = L"";
    if (ultimoPunto != wstring::npos) {
        extension = nombreBase.substr(ultimoPunto);
        nombreBase = nombreBase.substr(0, ultimoPunto);
    }

    vector<wstring> patrones = {
        L" \\(\\d+\\)", L"\\(\\d+\\)", L" - copia", L"-copia",
        L" \\(copia\\)", L"\\(copia\\)", L" \\(ver \\d+\\)",
        L"\\(ver \\d+\\)", L" - Copy", L"-Copy", L" Copy",
        L" copia", L"_copy", L"_copia"
    };

    for (const auto& patron : patrones) {
        wregex expresion(patron + L"$");
        nombreBase = regex_replace(nombreBase, expresion, L"");
    }

    return nombreBase + extension;
}

struct Nodo {
    string Nombreh;
    string Tamanioh;
    wstring NombreA;
    wstring NombreOriginal;
    wstring ubicacion;
    uint64_t tamanio;
    bool esDuplicado;
    Nodo* nodo;

    Nodo(string nh, string th, wstring na, wstring no, wstring ub, uint64_t tam, Nodo* siguiente) :
        Nombreh(nh), Tamanioh(th), NombreA(na), NombreOriginal(no), ubicacion(ub),
        tamanio(tam), esDuplicado(false), nodo(siguiente) {}
};

class ListaEnlazada {
private:
    Nodo* cabeza;
    map<string, vector<Nodo*>> gruposPorTamanio;

public:
    ListaEnlazada() : cabeza(nullptr) {}

    void agregar(const wstring& nombreArchivo, const wstring& ubicacion, uint64_t tamanio) {
        wstring nombreNormalizado = normalizarNombre(nombreArchivo);

        string nombreUTF8;
        for (size_t i = 0; i < nombreNormalizado.length(); ++i) {
            nombreUTF8 += (char)nombreNormalizado[i];
        }

        vector<uint8_t> entradaNombre;
        for (size_t i = 0; i < nombreUTF8.length(); ++i) {
            entradaNombre.push_back((uint8_t)nombreUTF8[i]);
        }   
        string Nombreh = md5(entradaNombre);

        vector<uint8_t> entradaTamano(8);
        for (size_t i = 0; i < 8; ++i) {
            entradaTamano[i] = (tamanio >> (i * 8)) & 0xFF;
        }
        string Tamanioh = md5(entradaTamano);

        Nodo* nuevo = new Nodo(Nombreh, Tamanioh, nombreArchivo, nombreNormalizado,
            ubicacion, tamanio, cabeza);
        cabeza = nuevo;

        gruposPorTamanio[Tamanioh].push_back(nuevo);

        vector<Nodo*>& grupo = gruposPorTamanio[Tamanioh];
        for (size_t i = 0; i < grupo.size() - 1; i++) {
            if (grupo[i]->Nombreh == nuevo->Nombreh) {
                grupo[i]->esDuplicado = true;
                nuevo->esDuplicado = true;
            }
        }
    }

    void mostrar() {
        Nodo* actual = cabeza;
        cout << "\nLista de archivos:\n";

        while (actual) {
            wcout << L"\nNombre Original: " << actual->NombreA
                << L"\nNombre Normalizado: " << actual->NombreOriginal
                << L"\nUbicacion: " << actual->ubicacion
                << L"\nTamano: " << actual->tamanio << L" bytes"
                << L"\nHash Nombre Normalizado: " << actual->Nombreh.c_str()
                << L"\nHash Tamano: " << actual->Tamanioh.c_str();

            if (actual->esDuplicado) {
                wcout << L"\n** ARCHIVO DUPLICADO **";
            }
            wcout << L"\n----------------------------------------\n";
            actual = actual->nodo;
        }
    }

    void mostrarDuplicados() {
        cout << "\nArchivos duplicados encontrados:\n";
        bool hayDuplicados = false;

        for (const auto& grupo : gruposPorTamanio) {
            const vector<Nodo*>& archivos = grupo.second;
            bool grupoDuplicados = false;

            for (size_t i = 0; i < archivos.size(); ++i) {
                if (archivos[i]->esDuplicado) {
                    if (!grupoDuplicados) {
                        cout << "\nGrupo de archivos duplicados:\n";
                        grupoDuplicados = true;
                        hayDuplicados = true;
                    }
                    wcout << L"\nNombre Original: " << archivos[i]->NombreA
                        << L"\nNombre Normalizado: " << archivos[i]->NombreOriginal
                        << L"\nUbicacion: " << archivos[i]->ubicacion
                        << L"\nTamano: " << archivos[i]->tamanio << L" bytes"
                        << L"\n----------------------------------------\n";
                }
            }
        }

        if (!hayDuplicados) {
            cout << "No se encontraron archivos duplicados.\n";
        }
    }

    ~ListaEnlazada() {
        while (cabeza) {
            Nodo* temp = cabeza;
            cabeza = cabeza->nodo;
            delete temp;
        }
    }
};

inline uint64_t getTamaniofile(const WIN32_FIND_DATAW& datosArchivo) {
    LARGE_INTEGER tamanio;
    tamanio.HighPart = datosArchivo.nFileSizeHigh;
    tamanio.LowPart = datosArchivo.nFileSizeLow;
    return static_cast<uint64_t>(tamanio.QuadPart);
}

inline void listar_y_hashear(const wstring& directorio, ListaEnlazada& lista) {
    wstring ruta = directorio + L"\\*";
    WIN32_FIND_DATAW datosFile;

    HANDLE hFind = FindFirstFileW(ruta.c_str(), &datosFile);

    if (hFind == INVALID_HANDLE_VALUE) {
        wcout << L"Error al abrir el directorio: " << directorio << L'\n';
        return;
    }

    do {
        const wstring nombreFile = datosFile.cFileName;

        if (nombreFile == L"." || nombreFile == L"..") {
            continue;
        }

        if (datosFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            listar_y_hashear(directorio + L"\\" + nombreFile, lista);
        }
        else {
            uint64_t tamanioFile = getTamaniofile(datosFile);
            lista.agregar(nombreFile, directorio, tamanioFile);
        }
    } while (FindNextFileW(hFind, &datosFile) != 0);

    FindClose(hFind);
}

#endif