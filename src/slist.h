#pragma once
#include <stdexcept>
#include <iostream>
#include <utility>
#include <memory>

// Узел односвязного списка.
// Хранит значение и уникальны указатель на следующий узел.
// У каждого узла - ровно один владелец: либо предыдущий узел, либо head списка.
template <typename T>
struct Node
{
    T value;
    std::unique_ptr<Node<T>> next; // владеет следующим узлом (или nullptr)

    // Конструктор для lvalue: копируем значение
    Node(T &val)
    {
        value = val;
        next = nullptr; // явно обнуляем (хотя unique_ptr и так nullptr по умолчанию)
    }

    // Конструктор для rvalue: перемещаем значение, если возможно
    Node(T &&val)
    {
        value = std::move(val);
        next = nullptr;
    }
};

// Односвязный список на unique_ptr - более "лёгкий" и эффективный,
// чем shared_ptr-версия, но без обратной связи (prev).
template <typename T>
class slist
{
private:
    std::unique_ptr<Node<T>> head; // владеет первым узлом (или nullptr)
    int length;                    // количество элементов

public:
    // Пустой список
    slist()
    {
        head = nullptr;
        length = 0;
    }

    // Move-конструктор: перехватывает ресурсы из другого списка
    slist(slist &&list)
    {
        head = std::move(list.head); // передаём владение цепочкой узлов
        length = list.length;

        // Оставляем исходный список пустым (но валидным)
        list.head = nullptr;
        list.length = 0;
    }

    // Деструктор не нужен: unique_ptr сам удалит все узлы
    ~slist() = default;

    // Вставка в конец: версия для lvalue (копирование)
    void push_back(T &value)
    {
        // Создаём новый узел (владение у newNode - unique_ptr)
        std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(value);

        if (head == nullptr)
        {
            // Список был пуст - head берёт владение
            head = std::move(newNode);
            length++;
            return;
        }

        // Идём по списку с помощью сырых указателей (get())
        // Не владеем узлами при обходе
        Node<T> *cur = head.get();
        while (cur->next != nullptr)
        {
            cur = cur->next.get();
        }

        // Присоединяем новый узел: передаём владение через move
        cur->next = std::move(newNode);
        length++;
    }

    // Вставка в конец: версия для rvalue (перемещение)
    void push_back(T &&value)
    {
        std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(std::move(value));
        if (head == nullptr)
        {
            head = std::move(newNode);
            length++;
            return;
        }

        Node<T> *cur = head.get();
        while (cur->next != nullptr)
        {
            cur = cur->next.get();
        }

        cur->next = std::move(newNode);
        length++;
    }

    // Печать списка (для отладки)
    void print()
    {
        Node<T> *cur = head.get(); // сырой указатель для обхода
        while (cur != nullptr)
        {
            std::cout << cur->value << ' ';
            cur = cur->next.get();
        }
        std::cout << '\n';
    }

    // Возвращает длину списка
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
        Node<T> *cur = head.get();
        while (cnt < index)
        {
            cur = cur->next.get();
            cnt++;
        }
        return cur->value; // возвращаем ссылку - можно изменять
    }

    // Вставка в произвольную позицию: перемещение
    void insert(T &&value, int position)
    {
        if (position > length || position < 0)
            throw std::out_of_range("Index out of range");

        std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(std::move(value));

        if (position == 0)
        {
            // Вставка в начало: новый узел берёт на себя старый head
            newNode->next = std::move(head);
            head = std::move(newNode);
            length++;
            return;
        }

        // Ищем узел ДО позиции вставки
        int cnt = 0;
        Node<T> *cur = head.get();
        while (cnt < position - 1)
        {
            cur = cur->next.get();
            cnt++;
        }

        // Вставляем: аккуратно передаём владение через move
        newNode->next = std::move(cur->next); // отрываем старый next
        cur->next = std::move(newNode);       // прикрепляем новый узел
        length++;
    }

    // Вставка в произвольную позицию: копирование
    void insert(T &value, int position)
    {
        if (position > length || position < 0)
            throw std::out_of_range("Index out of range");

        std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(value);
        if (position == 0)
        {
            newNode->next = std::move(head);
            head = std::move(newNode);
            length++;
            return;
        }

        int cnt = 0;
        Node<T> *cur = head.get();
        while (cnt < position - 1)
        {
            cur = cur->next.get();
            cnt++;
        }

        newNode->next = std::move(cur->next);
        cur->next = std::move(newNode);
        length++;
    }

    // Удаление по индексу
    void erase(int position)
    {
        if (position >= length || position < 0)
            throw std::out_of_range("Index out of range");

        if (position == 0)
        {
            // Удаляем голову: передаём владение next новому head
            head = std::move(head->next);
            length--;
            return;
        }

        // Ищем узел перед удаляемым
        int cnt = 0;
        Node<T> *cur = head.get();
        while (cnt < position - 1)
        {
            cur = cur->next.get();
            cnt++;
        }

        // "Перепрыгиваем" через удаляемый узел:
        // cur->next теперь владеет тем, чем владел cur->next->next
        cur->next = std::move(cur->next->next);
        length--;
    }

    // Move-присваивание
    slist &operator=(slist &&list)
    {
        if (this == &list)
            return *this;

        head = std::move(list.head);
        length = list.length;

        list.head = nullptr;
        list.length = 0;
        return *this;
    }

    // Copy-присваивание: поэлементное копировани
    slist &operator=(slist &list)
    {
        if (this == &list)
            return *this;

        // Очищаем текущий список
        head.reset();
        length = 0;

        // Копируем по одному элементу (используем push_back)
        Node<T> *cur = list.head.get();
        while (cur != nullptr)
        {
            push_back(cur->value); // вызовет копирующую версию push_back
            cur = cur->next.get();
        }
        return *this;
    }

    // Итератор: использует сырой указатель
    // не владеет узлами - только обходит их.
    class Iterator
    {
        Node<T> *ptr; // сырой указатель на текущий узел

    public:
        Iterator(Node<T> *p) : ptr(p) {}

        T &operator*() { return ptr->value; }
        T &get() { return ptr->value; }

        Iterator &operator++()
        {
            if (ptr != nullptr)
                ptr = ptr->next.get(); // .get() - сырой указатель из unique_ptr
            return *this;
        }

        bool operator!=(const Iterator &other) { return ptr != other.ptr; }
    };

    Iterator begin() { return Iterator(head.get()); }
    Iterator end() { return Iterator(nullptr); }
};