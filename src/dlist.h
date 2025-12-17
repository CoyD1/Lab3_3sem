#pragma once
#include <stdexcept>
#include <iostream>
#include <utility>
#include <memory>

// Узел двусвязного списка.
// Хранит значение, указатель на следующий узел (shared_ptr - владеет им),
// и указатель на предыдущий (weak_ptr - не владеет, чтобы избежать циклических ссылок).
template <typename T>
struct DoubleNode
{
    T value;
    std::shared_ptr<DoubleNode<T>> next; // владеющий указатель на следующий узел
    std::weak_ptr<DoubleNode<T>> prev;   // невладеющий указатель на предыдущий узел

    // Конструктор для lvalue
    DoubleNode(T &val)
    {
        value = val;  // копируем значение
        next.reset(); // next = nullptr
        prev.reset(); // prev = nullptr (weak_ptr тоже можно reset'ить)
    }

    // Конструктор для rvalue
    DoubleNode(T &&val)
    {
        value = std::move(val); // перемещаем, если возможно (например, для string, vector)
        next.reset();
        prev.reset();
    }
};

// Двусвязный список на умных указателях.
// Поддерживает вставку, удаление, доступ по индексу и итерацию.
template <typename T>
class dlist
{
private:
    std::shared_ptr<DoubleNode<T>> head; // первый элемент списка (nullptr если пуст)
    int length;                          // текущее количество элементов

public:
    // Конструктор пустого списка
    dlist()
    {
        head = nullptr;
        length = 0;
    }

    // Move-конструктор: "перехватывает" ресурсы из другого списка
    dlist(dlist &&list)
    {
        head = std::move(list.head); // передаём владение узлами
        length = list.length;

        // Оставляем исходный список в валидном (но пустом) состоянии
        list.head = nullptr;
        list.length = 0;
    }

    // Деструктор не нужен: shared_ptr сам очистит память
    ~dlist() = default;

    // Вставка в конец: версия для lvalue (копирование)
    void push_back(T &value)
    {
        // Создаём новый узел с копией значения
        std::shared_ptr<DoubleNode<T>> newNode = std::make_shared<DoubleNode<T>>(value);
        if (head == nullptr)
        {
            head = newNode; // первый элемент
            length++;
            return;
        }

        // Идём до конца списка
        auto cur = head;
        while (cur->next)
        {
            cur = cur->next;
        }

        // Связываем новый узел
        newNode->prev = cur; // слабая ссылка на текущий последний
        cur->next = newNode; // текущий теперь указывает на новый
        length++;
    }

    // Вставка в конец: версия для rvalue (перемещение)
    void push_back(T &&value)
    {
        // Создаём узел, перемещая значение (если тип поддерживает move)
        std::shared_ptr<DoubleNode<T>> newNode = std::make_shared<DoubleNode<T>>(std::move(value));
        if (head == nullptr)
        {
            head = newNode;
            length++;
            return;
        }

        auto cur = head;
        while (cur->next)
        {
            cur = cur->next;
        }
        newNode->prev = cur;
        cur->next = newNode;
        length++;
    }

    // Простой вывод списка
    void print()
    {
        auto cur = head;
        while (cur != nullptr)
        {
            std::cout << cur->value << ' ';
            cur = cur->next;
        }
        std::cout << '\n';
    }

    // Возвращает текущую длину
    int size()
    {
        return length;
    }

    // Доступ по индексу (с проверкой границ)
    T &operator[](int index)
    {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");

        int cnt = 0;
        auto cur = head;
        while (cnt < index)
        {
            cur = cur->next;
            cnt++;
        }
        return cur->value; // возвращаем ссылку - можно менять значение
    }

    // Вставка в произвольную позицию
    void insert(T &&value, int position)
    {
        if (position > length || position < 0)
            throw std::out_of_range("Index out_of_range");

        // Создаём узел с перемещённым значением
        std::shared_ptr<DoubleNode<T>> newNode = std::make_shared<DoubleNode<T>>(std::move(value));

        if (position == 0)
        {
            // Вставка в начало
            if (head != nullptr)
            {
                head->prev = newNode; // старый head теперь ссылается назад
                newNode->next = head; // новый узел указывает на старый head
            }
            head = newNode;
            length++;
            return;
        }

        // Ищем узел ДО позиции вставки
        auto cur = head;
        int cnt = 0;
        while (cnt < position - 1)
        {
            cur = cur->next;
            cnt++;
        }

        // Вставляем между cur и cur->next
        newNode->next = cur->next;
        if (newNode->next != nullptr)
        {
            newNode->next->prev = newNode; // обновляем prev у следующего узла
        }
        newNode->prev = cur;
        cur->next = newNode;
        length++;
    }

    // Вставка в произвольную позицию: копирование
    void insert(T &value, int position)
    {
        // Аналогично, но копируем значение
        if (position > length || position < 0)
            throw std::out_of_range("Index out of range");

        std::shared_ptr<DoubleNode<T>> newNode = std::make_shared<DoubleNode<T>>(value);
        if (position == 0)
        {
            if (head != nullptr)
            {
                head->prev = newNode;
                newNode->next = head;
            }
            head = newNode;
            length++;
            return;
        }

        auto cur = head;
        int cnt = 0;
        while (cnt < position - 1)
        {
            cur = cur->next;
            cnt++;
        }

        newNode->next = cur->next;
        if (newNode->next != nullptr)
        {
            newNode->next->prev = newNode;
        }
        newNode->prev = cur;
        cur->next = newNode;
        length++;
    }

    // Удаление по индексу
    void erase(int position)
    {
        if (position >= length || position < 0)
            throw std::out_of_range("Index out of range");

        if (position == 0)
        {
            if (head->next == nullptr)
            {
                head.reset(); // список стал пустым
            }
            else
            {
                auto newHead = head->next;
                newHead->prev.reset(); // обрываем обратную ссылку
                head = newHead;
            }
            length--;
            return;
        }

        // Ищем узел перед удаляемым
        int cnt = 0;
        auto cur = head;
        while (cnt < position - 1)
        {
            cur = cur->next;
            cnt++;
        }

        // Пропускаем удаляемый узел
        cur->next = cur->next->next;
        if (cur->next != nullptr)
        {
            // Обновляем prev у нового следующего узла
            cur->next->prev = cur;
        }
        length--;
    }

    // Move-присваивание
    dlist &operator=(dlist &&list)
    {
        if (this == &list)
            return *this; // самоприсваивание

        head = std::move(list.head);
        length = list.length;

        list.head = nullptr;
        list.length = 0;
        return *this;
    }

    // Copy-присваивание: создаёт копию поэлементно
    dlist &operator=(dlist &list)
    {
        if (this == &list)
            return *this;

        // Очищаем текущий список
        head.reset();
        length = 0;

        // Копируем все элементы
        auto cur = list.head;
        while (cur != nullptr)
        {
            push_back(cur->value); // используем существующий push_back
            cur = cur->next;
        }
        return *this;
    }

    // Простой итератор (поддерживает range-based for)
    class Iterator
    {
        DoubleNode<T> *ptr; // сырой указатель на текущий узел

    public:
        Iterator(DoubleNode<T> *p) : ptr(p) {}

        T &operator*() { return ptr->value; } // разыменование: получаем значение
        T &get() { return ptr->value; }       // альтернативный способ

        // Префиксный инкремент: переходим к следующему узлу
        Iterator &operator++()
        {
            if (ptr != nullptr)
                ptr = ptr->next.get(); // .get()-  получаем сырой указатель из shared_ptr
            return *this;
        }

        // Сравнение: нужно для условия цикла for (it != end())
        bool operator!=(const Iterator &other) { return ptr != other.ptr; }
    };

    // Начало итерации
    Iterator begin() { return Iterator(head.get()); }
    // Конец итерации (nullptr)
    Iterator end() { return Iterator(nullptr); }
};