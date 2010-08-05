#include <iostream>
#include <ctime>
#include <vector>

using namespace std;

const int NUM_ARR = 10;
vector<int> arr(NUM_ARR), arr_copy(NUM_ARR);

void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void print_vec(int arr[], int num)
{
	for (int i=0; i<num; ++i)
		cout << " " << arr[i];
	cout << endl;
}

typedef void (&sort_algorithm)(int[], int);

void test_sort(sort_algorithm sort_alg)
{
	copy(arr_copy.begin(), arr_copy.end(), arr.begin());
	sort_alg(&arr[0], arr.size());
	print_vec(&arr[0], arr.size());
}

void sort_insert(int arr[], int num)
{
	for (int i=1; i<num; ++i)
	{
		int j;
		int tmp = arr[i];
		for (j=i; j>0 && arr[j-1]>tmp; --j)
			arr[j] = arr[j-1];
		arr[j] = tmp;
	}
}

void sort_shell(int arr[], int num)
{
	for (int inc=num/2; inc>0; inc/=2)
	{
		for (int i=inc; i<num; ++i)
		{
			int j;
			int tmp = arr[i];
			for (j=i; j>=inc && tmp<arr[j-inc]; j-=inc)
				arr[j] = arr[j-inc];
			arr[j] =  tmp;
		}
	}
}

void sort_shell2(int arr[], int num)
{
	for (int inc=num/2; inc>0; inc/=2)
	{
		for (int i=0; i<inc; ++i)
		{
			for (int j=inc+i; j<num; j+=inc)
			{
				int k;
				int tmp = arr[j];
				for (k=j; k>i && tmp<arr[k-inc]; k-=inc)
					arr[k] = arr[k-inc];
				arr[k] = tmp;
			}
		}
	}
}

void sort_shell2_opt(int arr[], int num)
{
	for (int inc=num/2; inc>0; inc/=2)
	{
		for (int j=inc; j<num; ++j)
		//for (int i=0; i<inc; ++i)
		{
			//for (int j=inc+i; j<num; j+=inc)
			{
				int k;
				int tmp = arr[j];
				for (k=j; k>=inc && tmp<arr[k-inc]; k-=inc)
					arr[k] = arr[k-inc];
				arr[k] = tmp;
			}
		}
	}
}

void sort_select(int arr[], int num)
{
	for (int i=0; i<num; ++i)
	{
		int p = i;
		for (int j=i+1; j<num; ++j) 
			if (arr[j]<arr[p])
				p = j;
		if (p!=i)
		{
			int tmp = arr[p];
			arr[p] = arr[i];
			arr[i] = tmp;
		}
	}
}

void sort_bubble(int arr[], int num)
{
	bool swapped;
	do 
	{
		swapped = false;
		for (int i=0; i<num-1; ++i)
			if (arr[i]>arr[i+1])
			{
				int tmp = arr[i];
				arr[i] = arr[i+1];
				arr[i+1] = tmp;
				swapped = true;
			}
	} while (swapped);
}

void quick_sort(int arr[], int beg, int end)
{
	if (end > beg)
	{
		int piv = arr[beg], k = beg+1, r=end;
		while (k<r)
		{
			if (arr[k] < piv)
				++k;
			else
				swap(&arr[k], &arr[r--]);
		}
		if (arr[k] < piv)
		{
			swap(&arr[k], &arr[beg]);
			quick_sort(arr, beg, k);
			quick_sort(arr, r, end);
		}
		else
		{
			if (end-beg==1) return;
			swap(&arr[--k], &arr[beg]);
			quick_sort(arr, beg, k);
			quick_sort(arr, r, end);
		}
	}
}

void sort_quick(int arr[], int num)
{
	quick_sort(arr, 0, num-1);
}

#define LeftChild(i) (2*(i)+1)

void precolate_down(int arr[], int i, int num)
{
	int child;
	int tmp = arr[i];
	for (; LeftChild(i)<num; i=child)
	{
		child = LeftChild(i);
		if (child!=num-1 && arr[child+1]>arr[child])
			child++;
		if (tmp < arr[child])
			arr[i] = arr[child];
		else
			break;
	}
	arr[i] = tmp;
}

void sort_heap(int arr[], int num)
{
	int i;
	for (i=num/2; i>=0; --i) // build heap
		precolate_down(arr, i, num);

	for (i=num-1; i>0; --i)
	{
		swap(&arr[0], &arr[i]); // delete max
		precolate_down(arr, 0, i);
	}
}

int main(int argc, char *argv[])
{
	srand(static_cast<int>(time(NULL)));
	for (int i=0; i<NUM_ARR; ++i)
		arr[i] = rand() % 100;
	copy(arr.begin(), arr.end(), arr_copy.begin());

	cout << "Original array:\n";
	print_vec(&arr[0], arr.size());

	cout << "Insertion sort:\n";
	test_sort(sort_insert);

	cout << "Selection sort:\n";
	test_sort(sort_select);

	cout << "Bubble sort:\n";
	test_sort(sort_bubble);

	cout << "Quick sort:\n";
	test_sort(sort_quick);

	cout << "Shell sort:\n";
	test_sort(sort_shell);

	cout << "Shell sort 2:\n";
	test_sort(sort_shell2);

	cout << "Shell sort 2 opt:\n";
	test_sort(sort_shell2_opt);

	cout << "Heap sort:\n";
	test_sort(sort_heap);
}