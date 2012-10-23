#!/usr/bin/python

from random import *;
import sys;

letters = "ABCDEFGHIJKLMNOPQRSTUV";
numbers = "1234567890";
surnames = ["Sventek", "Gay", "Johnson", "ODonnel", "Norman", "Williamson", "Chalmers", "Storer", "Cutts", "Balkind"];
titles = ["Mr.", "Dr.", "Prof.", "Sir"]
firstnames = ["Joe", "Joseph", "Simon", "Chris", "John", "Gethin", "Matthew", "Tim", "Quintin", "Jonathan"]
streetnames = ["Penny Lane", "Abbey Road", "Broadway", "Sunset Blvd"]
cities = ["London", "Glasgow", "Edinburgh", "Stirling", "Inverness"]

def randlist(l):
    return l[randint(0, len(l)-1)];

def postcode():
    p = letters+numbers;
    s = randlist(letters);
    for i in range(5):
        s += randlist(p);
    return s[:3] + ' ' + s[3:];

def generate(n):
    #generate a few post codes to use, so duplicates are more likely
    postcodes = [];
    for i in range(max(1, n/100)):
        postcodes += [postcode()];

    for i in range(n):
        #name
        print randlist(surnames)+",",
        print randlist(titles),
        print randlist(firstnames)
        
        #house number (maybe) and street name
        if randint(0, 100) != 0:
            print randint(1, 100),
        print randlist(streetnames)
        
        #postcode
        print randlist(postcodes),
        print randlist(cities)

if len(sys.argv) < 2:
    print "Usage:", sys.argv[0], "n"
else:
    generate(int(sys.argv[1]));
