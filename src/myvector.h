#pragma once

#include <stdexcept>
#include <iostream>
#include <utility>
#include <memory>

// Простой вектор на основе динамического массива и unique_ptr<T[]>.
// Поддерживает автоматическое расширение, вставку, удаление и итерацию.
template <typename T>
class myvector
{
private:
    std::unique_ptr<T[]> data; // владеющий указатель на динамический массив
    int capacity = 1;          // текущий размер выделенной памяти (макс. элементов, которые можно хранить без realloc)
    int length = 0;            // количество реально занятых элементов

    // Увеличивает ёмкость в 2 раз и копирует старые данные в новый массив
    void resize()
    {
        capacity *= 2;
        std::unique_ptr<T[]> newData(new T[capacity]); // новый массив
        // Копируем существующие переменные
        for (int i = 0; i < length; ++i)
        {
            newData[i] = data[i];
        }
        data = std::move(newData); // передаём владение новым массивом
    }

public:
    // Конструктор по умолчанию: выделяет массив из 1 элемента
    myvector()
    {
        capacity = 1;
        length = 0;
        data = std::unique_ptr<T[]>(new T[capacity]);
    }

    // Move-конструктор: перемещает массив и состояние из другого вектора
    myvector(myvector &&vect)
    {
        data = std::move(vect.data);
        capacity = vect.capacity;
        length = vect.length;

        // Обнуляем исходный вектор, чтобы он не удалил массив при деструкции
        vect.data = nullptr;
        vect.capacity = 0;
        vect.length = 0;
    }

    // Деструктор не нужен: unique_ptr сам удалит массив
    ~myvector() = default;

    // Возвращает количество элементов
    int size()
    {
        return length;
    }

    // Добавление в конец: копирующая версия (для lvalue)
    void push_back(T &value)
    {
        if (length == capacity)
        {
            resize(); // расширяем, если массив полон
        }
        data[length] = value;
        length++;
    }

    // Добавление в конец: перемещающая версия (для rvalue)
    void push_back(T &&value)
    {
        if (length == capacity)
        {
            resize();
        }
        data[length] = std::move(value);
        length++;
    }

    // Вставка в произвольную позицию: перемещающая версия
    void insert(T &&value, int position)
    {
        if (position > length || position < 0)
            throw std::out_of_range("Index out of range");

        if (length == capacity)
        {
            resize();
        }

        // Сдвигаем элементы вправо, начиная с конца
        for (int i = length; i > position; --i)
        {
            data[i] = data[i - 1];
        }

        data[position] = std::move(value);
        length++;
    }

    // Вставка в произвольную позицию: копирующая версия
    void insert(T &value, int position)
    {
        if (position > length || position < 0)
            throw std::out_of_range("Index out of range");

        if (length == capacity)
        {
            resize();
        }

        for (int i = length; i > position; --i)
        {
            data[i] = data[i - 1];
        }

        data[position] = value;
        length++;
    }

    // Удаление элемента по индексу
    void erase(int position)
    {
        if (position >= length || position < 0)
            throw std::out_of_range("Index out of range");

        // Сдвигаем элементы влево, затирая удаляемый
        for (int i = position; i < length - 1; ++i)
        {
            data[i] = data[i + 1];
        }

        length--;
    }

    // Доступ по индексу (с проверкой)
    T &operator[](int index)
    {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    // Печать всех элементов (для отладки)
    void print()
    {
        for (int i = 0; i < length; ++i)
        {
            std::cout << data[i] << ' ';
        }
        std::cout << '\n';
    }

    // Move-присваивание
    myvector &operator=(myvector &&vect)
    {
        if (this == &vect)
        {
            return *this;
        }

        data = std::move(vect.data);
        capacity = vect.capacity;
        length = vect.length;

        vect.data = nullptr;
        vect.capacity = 0;
        vect.length = 0;
        return *this;
    }

    // Copy-присваивание: глубокое копирование данных
    myvector &operator=(myvector &vect)
    {
        if (this == &vect)
        {
            return *this;
        }

        // Выделяем новый массив нужного размера
        std::unique_ptr<T[]> newData(new T[vect.capacity]);
        for (int i = 0; i < vect.length; ++i)
        {
            newData[i] = vect.data[i]; // копируем элементы
        }

        data = std::move(newData);
        capacity = vect.capacity;
        length = vect.length;

        return *this;
    }

    // Итератор
    class Iterator
    {
        T *ptr; // сырой указатель на текущий элемент массива

    public:
        Iterator(T *p) : ptr(p) {}

        T &operator*() { return *ptr; }
        T &get() { return *ptr; }

        Iterator &operator++()
        {
            ++ptr;
            return *this;
        }

        bool operator!=(const Iterator &other) const { return ptr != other.ptr; }
    };

    // Начало диапазона: первый элемент
    Iterator begin() { return Iterator(data.get()); }
    // Конец диапазона: один за последним элементом
    Iterator end() { return Iterator(data.get() + length); }
};