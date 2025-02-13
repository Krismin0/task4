#include "lab4.h"
#include <cstring>
#include <iostream>
#include <queue>
#include <vector>

// 26

class VirtualPage {
public:
  int number;
  bool R;
  bool M;

  VirtualPage(int n, bool R, bool M) : number(n), R(R), M(M) {}
};

int operations_counter = 0;
const int pages_count = 14;
VirtualPage *tab[pages_count];
std::queue<int> second_chance_queue;

void timer_interrupt() {
  for (int i = 0; i < pages_count; ++i) {
    if (tab[i])
      tab[i]->R = 0;
  }
}

void log_current_table_state() {
  for (int i = 0; i < pages_count - 1; ++i) {
    if (tab[i])
      std::cout << tab[i]->number;
    else
      std::cout << '#';
    std::cout << " ";
  }

  if (tab[pages_count - 1])
    std::cout << tab[pages_count - 1]->number;
  else
    std::cout << "#";
  std::cout << std::endl;
}

void do_second_chance(int is_write, int n) {
  for (int i = 0; i < pages_count; ++i) {
    if (!tab[i]) {
      // вставка страницы на свободное место
      tab[i] = new VirtualPage(n, 1, is_write);
      second_chance_queue.push(i);
      return;
    } else if (tab[i]->number == n) {
      // страница уже есть в таблице
      tab[i]->R = true;
      return;
    }
  }

  while (true) {
    // берём страницу из головы очереди
    int indexToReplace = second_chance_queue.front();
    second_chance_queue.pop();
    if (!tab[indexToReplace]->R) {
      // бит использования не установлен - страница заменяется
      delete tab[indexToReplace];
      tab[indexToReplace] = new VirtualPage(n, 1, is_write);
      second_chance_queue.push(indexToReplace);
      return;
    } else {
      // Если при возникновении ошибки отсутствия страницы в памяти у страницы,
      // находящейся в голове очереди, бит использования R установлен (т.е.
      // равен 1), то этот бит сбрасывается в ноль, а сама страница перемещается
      // в конец очереди.
      tab[indexToReplace]->R = 0;
      second_chance_queue.push(indexToReplace);
    }
  }
}

void do_nru(int is_write, int n) {
  for (int i = 0; i < pages_count; ++i) {
    if (!tab[i]) {
      // вставка страницы на свободное место
      tab[i] = new VirtualPage(n, 1, is_write);
      return;
    } else if (tab[i]->number == n) {
      // страница уже есть в таблице
      tab[i]->R = true;
      if (!tab[i]->M)
        tab[i]->M = is_write;
      return;
    }
  }

  std::vector<int> replace_candidates;
  int min_page_class = 5, page_class;
  for (int i = 0; i < pages_count; ++i) {
    // подсчёт класса страницы
    if (tab[i]->M && !tab[i]->R)
      page_class = 1;
    else if (tab[i]->R && !tab[i]->M)
      page_class = 2;
    else if (tab[i]->R && tab[i]->M)
      page_class = 3;
    else
      page_class = 0;

    // обнаружена страница с меньшим классом - предыдущие страницы можно удилить
    // из списка кандидатов на замену
    if (page_class < min_page_class) {
      replace_candidates.clear();
      min_page_class = page_class;
    }

    if (min_page_class == page_class) {
      replace_candidates.push_back(i);
    }
  }

  // Удаляют страницу, относящуюся к классу с наименьшим номером.
  // Если таких страниц несколько, одну из них выбирают случайным образом.
  int replaceIndex =
      replace_candidates[uniform_rnd(0, replace_candidates.size() - 1)];
  delete tab[replaceIndex];

  tab[replaceIndex] = new VirtualPage(n, 1, is_write);
}

int main(int argc, char *argv[]) {
  int is_write, virtual_page_number;

  while (std::cin >> is_write >> virtual_page_number) {
    if (argv[1][0] == '1') {
      // second chance
      do_second_chance(is_write, virtual_page_number);
    } else {
      // nru
      do_nru(is_write, virtual_page_number);
    }

    // прерывание таймера каждые 5 операций
    if (++operations_counter % 5 == 0)
      timer_interrupt();

    log_current_table_state();
  }

  for (int i = 0; i < pages_count; ++i) {
    delete tab[i];
  }
}
