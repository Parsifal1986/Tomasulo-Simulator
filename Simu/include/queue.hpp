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
    iterator(Queue *q, int p, int s) : queue(q), pos(p), start(s), count(0) {}

    Data &operator*() const { return queue->queue[(start + pos) % CAPACITY]; }

    iterator &operator++() {
      pos = (pos + 1) % CAPACITY;
      count++;
      return *this;
    }

    bool operator!=(const iterator &other) const {
      return count != other.count;
    }

    int num() const {
      return pos;
    }

  private:
    Queue *queue;
    int pos;
    int start;
    int count;
  };

  iterator begin() { return iterator(this, head, head); }
  iterator end() { return iterator(this, tail, head); }

private:
  int head = 0, tail = 0, size = 0;
  Data queue[CAPACITY];
};

} // namespace parsifal_tools

#endif // QUEUE_HPP