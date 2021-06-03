#!/usr/sbin/perl

if ($ARGV[0] =~ m/^(-){0,1}(0|([1-9][0-9]{0,}))(\.[0-9]{0,}){0,1}([eE][+\-]?[0-9]{1,}){0,1}$/) {
	print "match \n";
} else {
	print "doesn't match\n";
}

