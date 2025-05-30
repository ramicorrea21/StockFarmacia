#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio> 

using namespace std;

namespace Stock {
     
    // Estructura principal
    struct Producto {
        string nombre = "";
        int codigo = 0;
        float precio = 0.0;
        int stock = 0;
        int activo = 1;
    };

    Producto productos[100];

    // Verifica si el archivo está vacío
    bool esta_vacio(string& path) {
        ifstream archivoEntrada(path);
        if (archivoEntrada.is_open())
            return archivoEntrada.peek() == ifstream::traits_type::eof();
        return false;
    }

    // Serializa un producto (convierte a string) en el orden: nombre, codigo, precio, stock, activo
    string serializar(const Producto& producto) {
        return producto.nombre + "," +
            to_string(producto.codigo) + "," +
            to_string(producto.precio) + "," +
            to_string(producto.stock) + "," +
            to_string(producto.activo);
    }

    // Parsea una línea del archivo a un objeto Producto (asumiendo el orden: nombre, codigo, precio, stock, activo)
    Producto parsearProducto(const string& linea) {
        stringstream ss(linea);
        Producto producto;
        getline(ss, producto.nombre, ',');
        string codigoStr, precioStr, stockStr, activoStr;
        getline(ss, codigoStr, ',');
        getline(ss, precioStr, ',');
        getline(ss, stockStr, ',');
        getline(ss, activoStr, ',');

        producto.codigo = stoi(codigoStr);
        producto.precio = stof(precioStr);
        producto.stock = stoi(stockStr);
        producto.activo = (activoStr == "1" || activoStr == "true");
        return producto;
    }

    // Función para recuperar los productos que están en el archivo
    int recuperarProductos(string path) {
        ifstream archivoProductos(path);
        int contador = 0;

        if (archivoProductos.is_open()) {
            string linea;
            while (getline(archivoProductos, linea) && contador < 100) {
                productos[contador] = parsearProducto(linea);
                contador++;
            }
            archivoProductos.close();
        }
        else {
            cout << "Error al abrir el archivo " << path << endl;
        }
        return contador;
    }

    // Función que se encarga de pasar los productos al archivo (utilizando serializar)
    void cargarProductosEnArchivo(Producto productos[100], int inicio, int n, string path) {
        ofstream ArchivoProductos(path, ios::app);
        if (ArchivoProductos.is_open()) {
            for (int i = inicio; i < n; i++) {
                ArchivoProductos << serializar(productos[i]) << "\n";
            }
            ArchivoProductos.close();
        }
        else {
            cout << "Error al abrir el archivo " << path << endl;
        }
    }

    // Registra nuevos productos
    int registrarProductos(Producto productos[100], int n) {
        int nuevos;
        int maxNuevos = 100 - n;
        cout << "Cuantos productos desea ingresar (maximo " << maxNuevos << "): ";
        cin >> nuevos;

        // Manejo de error al ingresar la cantidad de productos 
        while (cin.fail() || nuevos < 1 || nuevos > maxNuevos) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: ingrese un numero valido entre 1 y " << maxNuevos << ": ";
            cin >> nuevos;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        for (int i = n; i < n + nuevos; i++) {
            cout << "Ingrese el nombre del producto " << (i + 1) << ": ";
            getline(cin, productos[i].nombre);

            cout << "Ingrese el codigo del producto " << (i + 1) << ": ";
            cin >> productos[i].codigo;

            cout << "Ingrese el precio del producto " << (i + 1) << ": ";
            cin >> productos[i].precio;

            cout << "Ingrese el stock del producto " << (i + 1) << ": ";
            cin >> productos[i].stock;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        cargarProductosEnArchivo(productos, n, n + nuevos, "productos.txt");

        return n + nuevos;
    }

    // Función que verifica si existe un producto activo con el código ingresado en el archivo
    bool existeProductoConCodigo(int codigo, const string& path) {
        ifstream archivoEntrada(path);
        string linea;
        while (getline(archivoEntrada, linea)) {
            Producto producto = parsearProducto(linea);
            if (producto.codigo == codigo && producto.activo) {
                archivoEntrada.close();
                return true;
            }
        }
        archivoEntrada.close();
        return false;
    }

    // Baja de producto: marca el producto como inactivo, filtra solo los activos y actualiza el array
    int bajaDeProducto(int codigo, const string& path, Producto productos[100], int n) {
        if (!existeProductoConCodigo(codigo, path)) {
            cout << "No se encontro un producto activo con el codigo " << codigo << ".\n";
            return n; // Retorna el mismo número de productos (porque no se elimino ninguno)
        }

        ifstream archivoEntrada(path);
        ofstream archivoSalida("temp.txt");
        string linea;
        bool productoEliminado = false;

        while (getline(archivoEntrada, linea)) {
            Producto producto = parsearProducto(linea);

            // Si es el producto con el codigo a dar de baja, se marca como inactivo
            if (producto.codigo == codigo && producto.activo) {
                producto.activo = 0;
                productoEliminado = true;
            }

            // Solo se escriben en temp.txt los activos
            if (producto.activo) {
                archivoSalida << serializar(producto) << "\n";
            }
        }

        archivoEntrada.close();
        archivoSalida.close();

        // reemplaza el archivo original por el temp.txt (que solo tiene los activos)
        remove("productos.txt");
        rename("temp.txt", "productos.txt");

        if (productoEliminado) {
            cout << "El producto con codigo " << codigo << " ha sido dado de baja.\n";

            // actualizar el array que utilizamos en el programa, para que se impacte el eliminado en el resto de la app
            int nuevoN = recuperarProductos(path);
            return nuevoN;
        }

        return n; 
    }

    //Modificacion de STOCK de un producto
    int modificarStock(int codigo, int nuevoStock, const string& path, Producto productos[100], int n) {
        if (!existeProductoConCodigo(codigo, path)) {
            cout << "No se encontro un producto con el codigo " << codigo << ".\n";
            return n; // Retorna el mismo número si no se encontró
        }

        ifstream archivoIn("productos.txt");
        ofstream archivoOut("temp.txt");
        string linea;
        bool modificado = false;

        while (getline(archivoIn, linea)) {
            Producto producto = parsearProducto(linea);
            if (producto.codigo == codigo) {
                producto.stock = nuevoStock;
                modificado = true;
            }
            archivoOut << serializar(producto) << "\n";
        }
        archivoIn.close();
        archivoOut.close();
        remove("productos.txt");
        rename("temp.txt", "productos.txt");

        if (modificado) {
            cout << "Stock modificado correctamente." << endl;
            // Actualizar el array en memoria
            int nuevoN = recuperarProductos(path);
            return nuevoN;
        }

        return n;
    }

    //Modificacion de PRECIO de un producto
    int modificarPrecio(int codigo, float nuevoPrecio, const string& path, Producto productos[100], int n) {
        if (!existeProductoConCodigo(codigo, path)) {
            cout << "No se encontro un producto con el codigo " << codigo << ".\n";
            return n; // Retorna el mismo número si no se encontró
        }

        ifstream archivoIn("productos.txt");
        ofstream archivoOut("temp.txt");
        string linea;
        bool modificado = false;

        while (getline(archivoIn, linea)) {
            Producto producto = parsearProducto(linea);
            if (producto.codigo == codigo) {
                producto.precio = nuevoPrecio;
                modificado = true;
            }
            archivoOut << serializar(producto) << "\n";
        }
        archivoIn.close();
        archivoOut.close();
        remove("productos.txt");
        rename("temp.txt", "productos.txt");

        if (modificado) {
            cout << "Precio modificado correctamente." << endl;
            // Actualizar el array en memoria
            int nuevoN = recuperarProductos(path);
            return nuevoN;
        }

        return n;
    }

    // Muestra la lista de productos
    void mostrarProductos(Producto productos[100], int n) {
        for (int i = 0; i < n; i++) {
            cout << "\n-------------------------------------------\n";
            cout << "Producto " << (i + 1)
                << "\nNombre: " << productos[i].nombre
                << "\nCodigo: " << productos[i].codigo
                << "\nPrecio: " << productos[i].precio
                << "\nStock: " << productos[i].stock;
            if (!productos[i].activo) {
                cout << "\nEstado: Inactivo";
            }
            cout << "\n-------------------------------------------\n";
        }
    }

    // Filtra productos cuyo stock es menor a 10
    void filtrarPorStockMenorAdiez(Producto productos[100], int n) {
        for (int i = 0; i < n; i++) {
            if (productos[i].stock < 10) {
                cout << "-------------------------------------------\n";
                cout << "Producto " << (i + 1)
                    << "\nNombre: " << productos[i].nombre
                    << "\nCodigo: " << productos[i].codigo
                    << "\nPrecio: " << productos[i].precio
                    << "\nStock: " << productos[i].stock << "\n";
                cout << "-------------------------------------------\n";
            }
        }
    }

    // Muestra el valor total del inventario
    void valorTotalInventario(Producto productos[100], int n) {
        float valorTotal = 0.0;
        for (int i = 0; i < n; i++) {
            valorTotal += productos[i].precio * productos[i].stock;
        }
        cout << "El valor total del inventario es: " << valorTotal << "\n";
    }

    // Menú principal
    void mostrarMenu(Producto productos[100], int& n) {
        int opcion=0, opcion2=0;
        do {
            cout << "\n----Menu principal de stock de farmacia----\n";
            cout << "1. Mostrar productos\n";
            cout << "2. Filtrar por stock menor a 10\n";
            cout << "3. Mostrar total del inventario\n";
            cout << "4. Cargar mas productos\n";
            cout << "5. Eliminar un producto\n";
            cout << "6. Modificar un producto\n";
            cout << "0. Salir\n";
            cin >> opcion;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (opcion) {
            case 1:
                mostrarProductos(productos, n);
                break;
            case 2:
                filtrarPorStockMenorAdiez(productos, n);
                break;
            case 3:
                valorTotalInventario(productos, n);
                break;
            case 4:
                n = registrarProductos(productos, n);
                break;
            case 5: {
                int codigo, respuesta;
                cout << "\nIngrese el codigo del producto que desea eliminar: ";
                cin >> codigo;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Se procedera a dar de baja el producto con codigo " << codigo << ".\n";
                //Nos aseguramos de que el usuario realmente desee eliminarlo.
                cout << "Esta seguro que desea eliminar este producto? Ingrese 1 en caso de SI." << endl;
                cin >> respuesta;
                if (respuesta==1)
                {
                    n = bajaDeProducto(codigo, "productos.txt", productos, n);
                }
                else
                {
                    cout << "El producto de codigo: " << codigo << " no sera eliminado." << endl;
                }
                break;
            }
            case 6: {
                int codigo;
                cout << "Ingrese el codigo del producto que desea modificar: ";
                cin >> codigo;
                cout << "\n----------¿Que desea modificar?----------" << endl;
                cout << "1. Stock." << endl;
                cout << "2. Precio." << endl;
                cout << "3. Volver al menu principal." << endl;
                cin >> opcion2;

                switch (opcion2) {
                case 1: {
                    int nuevoStock;
                    cout << "Ingrese el stock nuevo del producto: ";
                    cin >> nuevoStock;
                    // Actualizar n con el valor retornado
                    n = modificarStock(codigo, nuevoStock, "productos.txt", productos, n);
                    break;
                }
                case 2: {
                    float nuevoPrecio;
                    cout << "Ingrese el nuevo precio del producto: ";
                    cin >> nuevoPrecio;
                    // Actualizar n con el valor retornado
                    n = modificarPrecio(codigo, nuevoPrecio, "productos.txt", productos, n);
                    break;
                }
                default:
                    cout << "Volviendo al menu principal..." << endl;
                    break;
                }
                break;
            }
            case 0:
                break;
            default:
                cout << "Ingrese una opcion valida, por favor.\n";
                break;
            }
        } while (opcion != 0);
    }

}

int main() {
    string path = "productos.txt";
    int n = 0;

    // Si el archivo está vacío se cargan productos desde la posición 0, de lo contrario se recuperan los existentes.
    if (Stock::esta_vacio(path)) {
        cout << "No hay productos registrados, ingrese nuevos productos:\n";
        n = Stock::registrarProductos(Stock::productos, 0);
    }
    else {
        n = Stock::recuperarProductos(path);
        cout << "Se han cargado " << n << " productos desde el archivo.\n";
    }

    Stock::mostrarMenu(Stock::productos, n);
    system("pause");
    return 0;
}
