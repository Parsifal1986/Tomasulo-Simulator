#ifndef QUEUE_HPP
#define QUEUE_HPP

namespace parsifal_tools {

template <typename Data, int CAPACITY> class Queue {
public:
  Data &Front() {
    if (Empty()) {
      return queue[head];
    }
    return queue[head];
  }

  Data &Back() {
    if (Empty()) {
      return queue[head];
    }
    return queue[tail];
  }

  bool Empty() { return size == 0; }

  int Size() { return size; }

  void Clear() {
    head = 0;
    tail = 0;
    size = 0;
  }

  bool PushBack(const Data data) {
    if (Size() == CAPACITY) {
      return false;
    }
    queue[tail] = data;
    tail = (tail + 1) % CAPACITY;
    size++;
    return true;
  }

  bool PopFront() {
    if (Empty()) {
      return false;
    }
    head = (head + 1) % CAPACITY;
    size--;
    return true;
  }

  Data &At(int pos) {
    return queue[pos];
  }

  class iterator {
  public:
    iterator(Queue *q, int p) : queue(q), pos(p) {}

    Data &operator*() const { return queue->queue[pos]; }

    iterator &operator++() {
      pos = (pos + 1) % CAPACITY;
      return *this;
    }

    bool operator!=(const iterator &other) const {
      return queue != other.queue || pos != other.pos;
    }

    int num() const {
      return pos;
    }

  private:
    Queue *queue;
    int pos;
  };

  iterator begin() { return iterator(this, head); }
  iterator end() { return iterator(this, tail); }

private:
  int head = 0, tail = 0, size = 0;
  Data queue[CAPACITY];
};

} // namespace parsifal_tools

#endif // QUEUE_HPP