#!/usr/bin/python

from random import *;
import sys;

letters = "ABCDEFGHIJKLMNOPQRSTUV";
numbers = "1234567890";
surnames = ["Sventek", "Smith", "Calder", "Dickman", "Moore", "Fletcher", "Doe", "Gay", "Johnson", "ODonnel", "James", "English", "Storer", "Cutts", "Macauley", "Davies"];
titles = ["Mr.", "Dr.", "Prof.", "Sir"]
firstnames = ["Joe", "Joseph", "John", "Jonathan", "James", "Jericho", "Jim", "Jones"]
streetnames = ["Penny Lane", "Abbey Road", "Broadway", "Sunset Blvd", "Cowgate"]
cities = ["London", "Glasgow", "Edinburgh", "Stirling", "Inverness", "Crianlarich", "Mallaig", "Fort William"]

def randlist(l):
    return l[randint(0, len(l)-1)];

def postcode():
    p = letters+numbers;
    s = randlist(letters);
    for i in range(5):
        s += randlist(p);
    return s[:3] + ' ' + s[3:];

def generate(n):
    postcodes = [];
    for i in range(max(1, n/1000)):
        postcodes += [postcode()];

    for i in range(n):
        #name
        print randlist(surnames)+",",
        print randlist(titles),
        print randlist(firstnames)
        #house number
        print randint(1, 100),
        print randlist(streetnames)
        #postcode
        print randlist(postcodes),
        print randlist(cities)

generate(int(sys.argv[1]));
