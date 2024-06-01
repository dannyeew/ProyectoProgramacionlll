#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace std;

struct NodoAVL {
    string palabra;
    map<string, string> traducciones;
    NodoAVL* izquierda;
    NodoAVL* derecha;
    int altura;

    NodoAVL(const string& palabra, const map<string, string>& traducciones)
        : palabra(palabra), traducciones(traducciones), izquierda(nullptr), derecha(nullptr), altura(1) {}
};

struct Usuario {
    string nombre;
    string contrasena;
    Usuario(const string& nombre, const string& contrasena) : nombre(nombre), contrasena(contrasena) {}
};

struct Historial {
    vector<tuple<string, string, string>> traduccionesRealizadas; // palabra original, idioma, traducción

    void registrarTraduccion(const string& palabra, const string& idioma, const string& traduccion) {
        traduccionesRealizadas.push_back(make_tuple(palabra, idioma, traduccion));
    }

    void guardarEnArchivo(const string& nombreArchivo) {
        ofstream archivo(nombreArchivo);
        if (archivo.is_open()) {
            for (const auto& entrada : traduccionesRealizadas) {
                archivo << get<0>(entrada) << ":" << get<1>(entrada) << ":" << get<2>(entrada) << endl;
            }
            archivo.close();
        } else {
            cout << "Error al abrir el archivo " << nombreArchivo << endl;
        }
    }
};

vector<Usuario> usuarios;
Usuario usuarioActual("", "");
Historial historial;

int altura(NodoAVL* nodo) {
    return nodo ? nodo->altura : 0;
}

int maximo(int a, int b) {
    return (a > b) ? a : b;
}

NodoAVL* rotacionDerecha(NodoAVL* y) {
    NodoAVL* x = y->izquierda;
    NodoAVL* T = x->derecha;

    x->derecha = y;
    y->izquierda = T;

    y->altura = maximo(altura(y->izquierda), altura(y->derecha)) + 1;
    x->altura = maximo(altura(x->izquierda), altura(x->derecha)) + 1;

    return x;
}

NodoAVL* rotacionIzquierda(NodoAVL* x) {
    NodoAVL* y = x->derecha;
    NodoAVL* T = y->izquierda;

    y->izquierda = x;
    x->derecha = T;

    x->altura = maximo(altura(x->izquierda), altura(x->derecha)) + 1;
    y->altura = maximo(altura(y->izquierda), altura(y->derecha)) + 1;

    return y;
}

int obtenerBalance(NodoAVL* nodo) {
    return nodo ? altura(nodo->izquierda) - altura(nodo->derecha) : 0;
}

NodoAVL* insertar(NodoAVL* raiz, const string& palabra, const map<string, string>& traducciones) {
    if (raiz == nullptr)
        return new NodoAVL(palabra, traducciones);

    if (palabra < raiz->palabra)
        raiz->izquierda = insertar(raiz->izquierda, palabra, traducciones);
    else if (palabra > raiz->palabra)
        raiz->derecha = insertar(raiz->derecha, palabra, traducciones);
    else
        return raiz; // La palabra ya está en el árbol

    raiz->altura = 1 + maximo(altura(raiz->izquierda), altura(raiz->derecha));

    int balance = obtenerBalance(raiz);

    // Casos de desbalance
    if (balance > 1 && palabra < raiz->izquierda->palabra)
        return rotacionDerecha(raiz);

    if (balance < -1 && palabra > raiz->derecha->palabra)
        return rotacionIzquierda(raiz);

    if (balance > 1 && palabra > raiz->izquierda->palabra) {
        raiz->izquierda = rotacionIzquierda(raiz->izquierda);
        return rotacionDerecha(raiz);
    }

    if (balance < -1 && palabra < raiz->derecha->palabra) {
        raiz->derecha = rotacionDerecha(raiz->derecha);
        return rotacionIzquierda(raiz);
    }

    return raiz;
}

NodoAVL* nodoMinimoValor(NodoAVL* nodo) {
    NodoAVL* actual = nodo;
    while (actual->izquierda != nullptr)
        actual = actual->izquierda;
    return actual;
}

NodoAVL* eliminar(NodoAVL* raiz, const string& palabra) {
    if (raiz == nullptr)
        return raiz;

    if (palabra < raiz->palabra)
        raiz->izquierda = eliminar(raiz->izquierda, palabra);
    else if (palabra > raiz->palabra)
        raiz->derecha = eliminar(raiz->derecha, palabra);
    else {
        if (raiz->izquierda == nullptr || raiz->derecha == nullptr) {
            NodoAVL* temp = raiz->izquierda ? raiz->izquierda : raiz->derecha;

            if (temp == nullptr) {
                temp = raiz;
                raiz = nullptr;
            } else
                *raiz = *temp;

            delete temp;
        } else {
            NodoAVL* temp = nodoMinimoValor(raiz->derecha);
            raiz->palabra = temp->palabra;
            raiz->traducciones = temp->traducciones;
            raiz->derecha = eliminar(raiz->derecha, temp->palabra);
        }
    }

    if (raiz == nullptr)
        return raiz;

    raiz->altura = 1 + maximo(altura(raiz->izquierda), altura(raiz->derecha));

    int balance = obtenerBalance(raiz);

    if (balance > 1 && obtenerBalance(raiz->izquierda) >= 0)
        return rotacionDerecha(raiz);

    if (balance > 1 && obtenerBalance(raiz->izquierda) < 0) {
        raiz->izquierda = rotacionIzquierda(raiz->izquierda);
        return rotacionDerecha(raiz);
    }

    if (balance < -1 && obtenerBalance(raiz->derecha) <= 0)
        return rotacionIzquierda(raiz);

    if (balance < -1 && obtenerBalance(raiz->derecha) > 0) {
        raiz->derecha = rotacionDerecha(raiz->derecha);
        return rotacionIzquierda(raiz);
    }

    return raiz;
}

void cargarDesdeArchivo(NodoAVL*& raiz, const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo " << nombreArchivo << endl;
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        istringstream iss(linea);
        string palabra;
        iss >> palabra;

        map<string, string> traducciones;
        string segmento;
        while (iss >> segmento) {
            size_t pos = segmento.find(':');
            if (pos != string::npos) {
                string idioma = segmento.substr(0, pos);
                string traduccion = segmento.substr(pos + 1);
                traducciones[idioma] = traduccion;
            }
        }
        raiz = insertar(raiz, palabra, traducciones);
    }

    archivo.close();
}

void guardarEnArchivo(NodoAVL* raiz, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo " << nombreArchivo << endl;
        return;
    }

    vector<NodoAVL*> nodos;
    NodoAVL* actual = raiz;

    while (actual || !nodos.empty()) {
        while (actual) {
            nodos.push_back(actual);
            actual = actual->izquierda;
        }

        actual = nodos.back();
        nodos.pop_back();

        archivo << actual->palabra;
        for (const auto& par : actual->traducciones) {
            archivo << " " << par.first << ":" << par.second;
        }
        archivo << endl;

        actual = actual->derecha;
    }

    archivo.close();
}

void reproducirTraduccion(const string& palabra) {
    string comando = "espeak \"" + palabra + "\"";
    system(comando.c_str());
}

void traducirPalabra(NodoAVL* nodo, const string& idioma, Historial& historial) {
    if (nodo->traducciones.find(idioma) != nodo->traducciones.end()) {
        string traduccion = nodo->traducciones[idioma];
        cout << "Traducción a " << idioma << ": " << traduccion << endl;
        historial.registrarTraduccion(nodo->palabra, idioma, traduccion); // Registrar la traducción en el historial
        reproducirTraduccion(traduccion); // Reproducir la traducción
    } else {
        cout << "Traducción a " << idioma << " no encontrada" << endl;
    }
}

void imprimirTraducciones(NodoAVL* raiz, string palabra, Historial& historial) {
    if (raiz == nullptr) {
        cout << "Palabra no encontrada" << endl;
        return;
    }

    NodoAVL* actual = raiz;
    while (actual != nullptr) {
        if (palabra == actual->palabra) {
            cout << "Traducciones disponibles:" << endl;
            for (const auto& par : actual->traducciones) {
                cout << par.first << endl;
            }

            string idioma;
            cout << "Ingrese el idioma al que desea traducir: ";
            cin >> idioma;

            traducirPalabra(actual, idioma, historial);
            return;
        } else if (palabra < actual->palabra) {
            actual = actual->izquierda;
        } else {
            actual = actual->derecha;
        }
    }

    cout << "Palabra no encontrada" << endl;
}

void cargarUsuariosDesdeArchivo(vector<Usuario>& usuarios, const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (archivo.is_open()) {
        string linea;
        while (getline(archivo, linea)) {
            istringstream iss(linea);
            string nombre, contrasena;
            getline(iss, nombre, ',');
            getline(iss, contrasena, ',');
            usuarios.emplace_back(nombre, contrasena);
        }
        archivo.close();
    }
}

void guardarUsuariosEnArchivo(const vector<Usuario>& usuarios, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo);
    if (archivo.is_open()) {
        for (const auto& usuario : usuarios) {
            archivo << usuario.nombre << "," << usuario.contrasena << endl;
        }
        archivo.close();
    }
}

void crearUsuario(vector<Usuario>& usuarios) {
    string nombre, contrasena;
    cout << "Ingrese nombre de usuario: ";
    cin >> nombre;
    cout << "Ingrese contraseña: ";
    cin >> contrasena;
    usuarios.emplace_back(nombre, contrasena);
    guardarUsuariosEnArchivo(usuarios, "usuarios.txt");
}

bool iniciarSesion(vector<Usuario>& usuarios, Usuario& usuarioActual) {
    string nombre, contrasena;
    cout << "Ingrese nombre de usuario: ";
    cin >> nombre;
    cout << "Ingrese contraseña: ";
    cin >> contrasena;
    for (const auto& usuario : usuarios) {
        if (usuario.nombre == nombre && usuario.contrasena == contrasena) {
            usuarioActual = usuario;
            return true;
        }
    }
    return false;
}

void gestionarUsuarios() {
    int opcion;
    do {
        cout << "1. Crear usuario\n2. Iniciar sesion\nSeleccione una opcion: ";
        cin >> opcion;
        switch (opcion) {
            case 1:
                crearUsuario(usuarios);
                break;
            case 2:
                if (iniciarSesion(usuarios, usuarioActual)) {
                    cout << "Inicio de sesion exitoso\n";
                    return;
                } else {
                    cout << "Usuario o contraseña incorrectos\n";
                }
                break;
            default:
                cout << "Opción no valida. Intente nuevamente.\n";
        }
    } while (opcion != 2);
}

void mostrarMenu() {
    cout << "Bienvenido al diccionario multilingue" << endl;
    cout << "1. Buscar traducciones de una palabra" << endl;
    cout << "2. Agregar una nueva palabra y sus traducciones" << endl;
    cout << "3. Eliminar una palabra y sus traducciones" << endl;
    cout << "4. Mostrar historial de traducciones" << endl;
    cout << "5. Encriptar una palabra" << endl;
    cout << "6. Desencriptar una palabra" << endl;
    cout << "7. Cerrar sesion" << endl;
    cout << "Seleccione una opcion: ";
}

string encriptar(string& palabra, NodoAVL*& raiz) {
    string resultado;
    for (char c : palabra) {
        if (isupper(c)) {
            if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
                resultado += 'U';
            } else {
                resultado += 'g';
            }
        } else if (islower(c)) {
            if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
                resultado += 'U';
            } else {
                resultado += 'm';
            }
        } else {
            resultado += c;
        }
    }

    // Eliminar la palabra del diccionario
    raiz = eliminar(raiz, palabra);

    return resultado;
}


string desencriptar(const string& palabra, NodoAVL*& raiz) {
    string resultado;
    for (char c : palabra) {
        if (c == 'U') {
            resultado += 'a'; // Suponiendo 'a' como vocal de ejemplo
        } else if (c == 'm') {
            resultado += 'b'; // Suponiendo 'b' como consonante minúscula de ejemplo
        } else if (c == 'g') {
            resultado += 'B'; // Suponiendo 'B' como consonante mayúscula de ejemplo
        } else {
            resultado += c;
        }
    }

    // Agregar la palabra desencriptada de nuevo al diccionario
    map<string, string> traducciones;
    string idioma, traduccion;
    char deseaAgregarMas;
    cout << "Ingrese el idioma de la traduccion: ";
    cin >> idioma;
    cout << "Ingrese la traduccion en " << idioma << ": ";
    cin >> traduccion;
    traducciones[idioma] = traduccion;
    raiz = insertar(raiz, resultado, traducciones);

    return resultado;
}



void registrarEncriptacion(const string& usuario, const string& palabraOriginal, const string& palabraEncriptada, bool encriptar) {
    ofstream archivo("encriptacion.txt", ios::app);
    if (archivo.is_open()) {
        archivo << (encriptar ? "Encriptado por" : "Desencriptado por") << usuario << ": " << palabraOriginal << " -> " << palabraEncriptada << endl;
        archivo.close();
    } else {
        cout << "Error al abrir el archivo de encriptaciones" << endl;
    }
}

void procesarOpcion(int opcion, NodoAVL*& raiz) {
    switch (opcion) {
        case 1: {
            string palabra;
            cout << "Ingrese la palabra a traducir: ";
            cin >> palabra;
            imprimirTraducciones(raiz, palabra, historial);
            break;
        }
        case 2: {
            string palabra;
            cout << "Ingrese la palabra a agregar: ";
            cin >> palabra;
            map<string, string> traducciones;
            string idioma, traduccion;
            char deseaAgregarMas;
            do {
                cout << "Ingrese el idioma de la traduccion: ";
                cin >> idioma;
                cout << "Ingrese la traduccion en " << idioma << ": ";
                cin >> traduccion;
                traducciones[idioma] = traduccion;
                cout << "Desea agregar otra traduccion? (S/N): ";
                cin >> deseaAgregarMas;
            } while (toupper(deseaAgregarMas) == 'S');
            raiz = insertar(raiz, palabra, traducciones);
            break;
        }
        case 3: {
            string palabra;
            cout << "Ingrese la palabra a eliminar: ";
            cin >> palabra;
            raiz = eliminar(raiz, palabra);
            break;
        }
        case 4: {
            historial.guardarEnArchivo("historial.txt");
            ifstream archivo("historial.txt");
            if (archivo.is_open()) {
                string linea;
                while (getline(archivo, linea)) {
                    cout << linea << endl;
                }
                archivo.close();
            }
            break;
        }
 		case 5: {
    string palabra;
    cout << "Ingrese la palabra a encriptar y eliminar del diccionario: ";
    cin >> palabra;
    string encriptada = encriptar(palabra, raiz);
    registrarEncriptacion(usuarioActual.nombre, palabra, encriptada, true);
    cout << "Palabra encriptada: " << encriptada << endl;
    break;
}

case 6: {
    string palabra;
    cout << "Ingrese la palabra a desencriptar y agregar al diccionario: ";
    cin >> palabra;
    string desencriptada = desencriptar(palabra, raiz);
    registrarEncriptacion(usuarioActual.nombre, palabra, desencriptada, false);
    cout << "Palabra desencriptada: " << desencriptada << endl;
    break;
}


        case 7:
            guardarEnArchivo(raiz, "diccionario.txt");
            cout << "Gracias por usar el diccionario multilingue. Hasta luego!" << endl;
            exit(0);
        default:
            cout << "Opción no válida. Por favor, seleccione una opción válida." << endl;
    }
}

int main() {
    NodoAVL* raiz = nullptr;
    cargarDesdeArchivo(raiz, "diccionario.txt");
    cargarUsuariosDesdeArchivo(usuarios, "usuarios.txt");

    gestionarUsuarios();

    int opcion;
    do {
        mostrarMenu();
        cin >> opcion;
        procesarOpcion(opcion, raiz);
    } while (opcion != 7);

    return 0;
}