function profile_test()
big_time_consume()
small_time_consume()

function big_time_consume()
s = 0;
for i=1:1e6
   s = s + i; 
end
sub_big_time_consume()
small_time_consume()

function sub_big_time_consume()
s = 0;
for i=1:1e5
   s = s + i; 
end

function small_time_consume()
s = 0;
for i=1:1e5
    s = s + i;
end