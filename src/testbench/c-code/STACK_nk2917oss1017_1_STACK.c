int recursion(int n);

int main(){
    int a = 0;
    recursion(a);
}

int recursion(int n){
    if(n <= 1000000){
        return recursion(n+1);
    }
    else{
        return n-1000000;
    }
}