#!/usr/bin/python

import random, string, sys, getopt, itertools, os

usage_string = "gentest.py -n <num> -o output"


MAX_REQUEST_VALUE = 100
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
    age = 1
    size = 0
    cur_top = 0

    def __init__(self, cache_size=None, cache_list=None):
        self.size = cache_size
        self.head = cache_list

class NODE:
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
    ptr = head
    while ptr and ptr.next:
        if (ptr.index == val.index):
            return ptr
        ptr = ptr.next
    if (ptr and ptr.index == val.index):
        return ptr
    return None

def clist_remove (head: NODE, toremove: NODE):
    ptr = head
    while ptr.next:
        if (ptr.next.index == toremove.index):
            ptr.next = ptr.next.next
    return ptr.next

def cache_node_freq (cache_node):
    return cache_node.freq

def GenerateAnswer(test):
    cache_size = int(test[0])
    test_list = [int(request) for request in test[2:]]
    
    lfuda = LFUDA(cache_size, None)

    for index in test_list:
        entry = NODE(index, None)
        found = clist_lookup(lfuda.head, entry)
        if (found):
            lfuda.hits += 1
            found.freq = lfuda.age + 1 * found.freq
        elif (lfuda.cur_top < lfuda.size):
            entry.freq = lfuda.age
            lfuda.head = push_front(lfuda.head, entry)
            lfuda.cur_top += 1
        else:
            toremove = lfuda.head
            lfuda.age = toremove.freq
            entry.freq = lfuda.age
            lfuda.head = push_front(lfuda.head, entry)
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