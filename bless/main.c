/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <stdlib.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>

int hash[256];
int bloom[256];

const int MAXNUM=1000;
const int threshold=3;
const int r=20;
int** reads;
int* a;

/*
 *  ======== taskFxn ========
 */

int** spliting (int* data)
{
	int i,j;
	System_printf("Counting...start,size = %d\n",sizeof(data));
	reads=(int**)malloc(sizeof(int*)*r);
	for(i=0;i<19;i++)
	{
		int t;
		t=rand()%50;

		*(reads+i)=(int*)malloc(sizeof(int)*(51+t));
		reads[i][0]=50+t;

	}
	*(reads+19)=(int*)malloc(sizeof(int)*(51));
	reads[19][0]=50;

	for(i=0;i<r;i++)
	{
		for(j=1;j<reads[i][0]+1;j++)
			reads[i][j]=a[i*50+j-1];
	}
	return reads;

}

int* counting (int** ref,int k)
{
	int i,j;
	System_printf("Counting...start,size = %d\n",sizeof(ref));
	for(i=0;i<r;i++)
	{
		for(j=1;j<ref[i][0]-k;j++)
			hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]++;
	}

	return hash;
}

int* bloom_set (int* b)
{
	int i;

	for(i=0;i<256;i++)
	{
		if(hash[i]>=threshold)
			b[i]=1;
	}

	return b;
}

int query_bloom (int index)
{
	return bloom[index];
}

int** correction (int** ref,int k)
{
	int i,j,l;
	System_printf("Correction...start,size = %d\n",sizeof(ref));

	for(i=0;i<r;i++)
	{
		for(j=1;j<ref[i][0]-k;j++)
		{
			if(!query_bloom(ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]))
			{
				for(l=0;l<4;l++)
				{
					if(hash[l*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]>=threshold)
					{
						hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]--;
						hash[l*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]++;
						ref[i][j]=l;
						System_printf("Reads %d-%d..be corrected\n",i,j);
						break;
					}
					else if(hash[ref[i][j]*64+l*16+ref[i][j+2]*4+ref[i][j+3]]>=threshold)
					{
						hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]--;
						hash[ref[i][j]*64+l*16+ref[i][j+2]*4+ref[i][j+3]]++;
						ref[i][j+1]=l;
						System_printf("Reads %d-%d..be corrected\n",i,j+1);
						break;
					}
					else if(hash[ref[i][j]*64+ref[i][j+1]*16+l*4+ref[i][j+3]]>=threshold)
					{
						hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]--;
						hash[ref[i][j]*64+ref[i][j+1]*16+l*4+ref[i][j+3]]++;
						ref[i][j+2]=l;
						System_printf("Reads %d-%d..be corrected\n",i,j+2);
						break;
					}
					else if(hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+l]>=threshold)
					{
						hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+ref[i][j+3]]--;
						hash[ref[i][j]*64+ref[i][j+1]*16+ref[i][j+2]*4+l]++;
						ref[i][j+3]=l;
						System_printf("Reads %d-%d..be corrected\n",i,j+3);
						break;
					}
				}
			}

		}
	}

    return ref;
}

Void taskFxn(UArg a0, UArg a1)
{
	a =(int*)malloc(sizeof(int)*MAXNUM);
	int i,j,sum=0;

	System_printf("size = %d\n",sizeof(a));
	for(i=0;i<256;i++)
	   hash[i]=0;

	for(i=0;i<256;i++)
	   bloom[i]=0;

	for(i=0;i<MAXNUM;i++)
	{
		a[i]=rand()%4;
		System_printf("%d",a[i]);
	}

	System_printf("\n");

	spliting(a);

    for(i=0;i<r;i++)
    {
    	System_printf("%d...READS\n",i);
    	for(j=1;j<reads[i][0]+1;j++)
    		System_printf("%d",reads[i][j]);
    	System_printf("\n");
    }

	counting(reads,4);

	for(i=0;i<256;i++)
	{
		System_printf("%d = %d\n",i,hash[i]);
		sum+=hash[i];
	}

	bloom_set(bloom);

	correction(reads,4);

	System_printf("Print result\n");

    for(i=0;i<r;i++)
    {
    	System_printf("%d...READS\n",i);
    	for(j=1;j<reads[i][0]+1;j++)
    		System_printf("%d",reads[i][j]);
    	System_printf("\n");
    }
    //Task_sleep(10);

    System_printf("exit taskFxn(),Counting...%d\n",sum);
}

/*
 *  ======== main ========
 */
Int main()
{ 
    Task_Handle task;
    Error_Block eb;

    System_printf("enter main()\n");

    Error_init(&eb);
    task = Task_create(taskFxn, NULL, &eb);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    BIOS_start();    /* does not return */
    System_printf("This line should not be in the buffer!\n");
    return(0);
}
