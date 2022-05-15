#!/usr/bin/python

import random, string, sys, getopt, itertools, os

usage_string = "gentest.py -n <num> -o output"


MAX_REQUEST_VALUE = 200
MAX_CACHE_SIZE = 100
MAX_REQUESTS_NUMBER = 10000

def GenerateTest(test_number, cache_size, requests_number):
    test_str = []
    test_str.append(str(cache_size))
    test_str.append(str(requests_number))

    for request in range(requests_number):
        test_str.append(str(random.randint(0, MAX_REQUEST_VALUE)))
    return test_str

def GenerateRandomTest(test_number):
    cache_size = random.randint(1, MAX_CACHE_SIZE)
    requests_number = random.randint(1, MAX_REQUESTS_NUMBER)
    test_str = GenerateTest(test_number, cache_size, requests_number)
    return test_str

class LFUDA:
    hits = 0
    age = 0
    size = 0
    cur_top = 0

    def __init__(self, cache_size=None, cache_list=None):
        self.size = cache_size
        self.head = cache_list

class NODE:
    weight = 0
    freq = 0
    index = 0
    def __init__(self, index=None, next=None):
        self.index = index
        self.next = next

def push_back(head: NODE, val: NODE) -> None:
    ptr = head
    while ptr.next:
        ptr = ptr.next
    ptr.next = val

def push_front(head: NODE, val: NODE):
    new_head = val
    new_head.next = head
    return new_head

def clist_lookup (head: NODE, val: NODE):
    if not head:
        return None
    if (head.index == val.index):
        return head
    ptr = head
    while ptr.next:
        if (ptr.index == val.index):
            return ptr
        ptr = ptr.next
    if (ptr.index == val.index):
        return ptr
    return None

def clist_remove (head: NODE, toremove: NODE):
    if (head == toremove):
        return head.next
    ptr = head
    while ptr.next:
        if (ptr.next.index == toremove.index):
            ptr.next = ptr.next.next
            break
        ptr = ptr.next
    return head

def clist_insert_after (last: NODE, toinsert: NODE):
    if not last:
        return None
    toinsert.next = last.next
    last.next = toinsert
    return toinsert

def clist_insert_sorted (head: NODE, toinsert: NODE):
    if not head:
        return toinsert
    if (head.weight >= toinsert.weight):
        toinsert.next = head
        return toinsert
    ptr = head
    while (ptr.next and ptr.next.weight < toinsert.weight):
        ptr = ptr.next
    clist_insert_after(ptr, toinsert)
    return head

def clist_get_to_remove (head: NODE):
    if not head:
        return None
    ptr = head
    while (ptr.next and ptr.next.weight == head.weight):
        ptr = ptr.next
    return ptr

def GenerateAnswer(test):
    cache_size = int(test[0])
    test_list = [int(request) for request in test[2:]]

    lfuda = LFUDA(cache_size, None)

    for index in test_list:
        entry = NODE(index, None)
        found = clist_lookup(lfuda.head, entry)
        if (found):
            lfuda.hits += 1
            found.freq += 1
            found.weight = lfuda.age + found.freq
            lfuda.head = clist_remove(lfuda.head, found)
            lfuda.head = clist_insert_sorted(lfuda.head, found)
        elif (lfuda.cur_top < lfuda.size):
            entry.freq = 1
            entry.weight = lfuda.age + 1
            lfuda.head = clist_insert_sorted(lfuda.head, entry)
            lfuda.cur_top += 1
        else:
            toremove = clist_get_to_remove(lfuda.head)
            lfuda.age = toremove.weight
            entry.freq = 1
            entry.weight = lfuda.age + 1
            lfuda.head = clist_remove(lfuda.head, toremove)
            lfuda.head = clist_insert_sorted(lfuda.head, entry)
    return lfuda.hits
           

class CmdArgs:
    number_of_tests = 0
    output_path = "./resources"

    def __init__(self):
        pass

def Generate_N_Random_Tests (cmd):
    for test_number in range(cmd.number_of_tests):
        test_str = GenerateRandomTest(test_number)
        with open (os.path.join(cmd.output_path, "test{}.dat".format(test_number)), 'w+') as test_file:
            test_file.write("{}".format(" ".join(test_str)))
        answ = GenerateAnswer(test_str)
        with open (os.path.join(cmd.output_path, "answ{}.dat".format(test_number)), "w+") as answer_file:
            answer_file.write("{}\n".format(answ))
        


def main (argv):
    cmd = CmdArgs()
    try:
        opts, args = getopt.getopt(argv, "hn:o:")
    except getopt.GetoptError:
        print(usage_string)
        sys.exit()
    for opt, arg in opts:
        if opt == "-h":
            print(usage_string)
            sys.exit()
        if opt in ("-o", "--output"):
            cmd.output_path = str(arg)
        if opt in ("-n", "--number"):
            cmd.number_of_tests = int(arg)
    
    Generate_N_Random_Tests (cmd)



main(sys.argv[1:])