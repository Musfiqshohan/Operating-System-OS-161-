import java.io.FileReader;
import java.util.*;


class Process implements Comparable<Process>{

    int id;
    int numberOfPages;
    int arrivalTime;
    ArrayList<Integer> memoryRef= new ArrayList<Integer>();
    int pageFault;
    int waitingTime;
    int executionTime;
    int totalTime;

    int turnaroundTime;
    int completionTime;



    void getPageFaultLRU(){


        ArrayList<Integer> pageTable= new ArrayList<Integer>();
        int tablesize=(numberOfPages-1)/3 +1;


        for(int i=0;i< memoryRef.size();i++){

            int currentmem= memoryRef.get(i);
            if(!pageTable.contains(currentmem)){

                if(pageTable.size()>=tablesize){
                    pageTable.remove(0);
                }
                pageTable.add(currentmem);
                pageFault++;
            }
            else{

                for(int j=0;j<pageTable.size();j++){
                    if(pageTable.get(j)==currentmem)
                        pageTable.remove(j);
                }

                pageTable.add(currentmem);
            }

        }



    }


    @Override
    public int compareTo(Process o) {

        if(this.arrivalTime-o.arrivalTime==0){
            int tot1=this.executionTime+this.waitingTime;
            int tot2=o.executionTime-o.waitingTime;
            return (tot1-tot2);

        }
        return (this.arrivalTime-o.arrivalTime);
    }
}


public class SJF {



    public double run(double fcfsturn) throws Exception{



        FileReader fin= new FileReader("input.txt");
        Scanner sc= new Scanner(fin);


        String line= sc.nextLine();
        line= line.replace(" ", "");

        StringTokenizer st= new StringTokenizer(line, ",");

        int numberOfProcess=Integer.parseInt(st.nextToken());
        int timeQuanta= Integer.parseInt(st.nextToken());




        Process pList[]= new Process[numberOfProcess];

        for (int i=0;i<numberOfProcess;i++){

            line=sc.nextLine();
            line= line.replace(" ", "");
            st= new StringTokenizer(line, ",");


            pList[i]=new Process();
            pList[i].id= Integer.parseInt(st.nextToken());
            pList[i].numberOfPages= Integer.parseInt(st.nextToken());
            pList[i].arrivalTime=Integer.parseInt(st.nextToken());



            while(st.hasMoreTokens()){


                pList[i].memoryRef.add((Integer.parseInt(st.nextToken()))/512);

            }

            pList[i].getPageFaultLRU();

            int pageFault=pList[i].pageFault;

            pList[i].waitingTime=pageFault*60;
            pList[i].executionTime= pList[i].memoryRef.size()*30;
            pList[i].totalTime=pList[i].waitingTime+ pList[i].executionTime;

        }

        Process backup[]= pList.clone();

        Arrays.sort(pList);



        PriorityQueue<Process>pq= new PriorityQueue<Process>(new ShortestJob());

        Process curProcess;
        int curTime=pList[0].arrivalTime;
        int totalWaiting=0,totalTurnAround=0;
        for(int i=0;i<numberOfProcess;i++){



            while(i<numberOfProcess && pList[i].arrivalTime<=curTime){
                pq.add(pList[i]);
                i++;
            }



            curProcess=pq.peek();
            pq.remove();
            curTime=Math.max(curTime,curProcess.arrivalTime);
            curTime+=curProcess.totalTime;
            totalWaiting+= (curTime-curProcess.arrivalTime-curProcess.executionTime);
            totalTurnAround+= (curTime- curProcess.arrivalTime);


            if(pq.isEmpty()==true && i<numberOfProcess && pList[i].arrivalTime>curTime) {
                pq.add(pList[i]);
                curTime=pList[i].arrivalTime;

            }else {
                i--;
            }


        }






        while(!pq.isEmpty()){
            curProcess=pq.peek();
            pq.remove();
            curTime=Math.max(curTime,curProcess.arrivalTime);

            curTime+=curProcess.totalTime;
            totalWaiting+= (curTime-curProcess.arrivalTime-curProcess.executionTime);
            totalTurnAround+= (curTime- curProcess.arrivalTime);

        }


        pList=backup;

        double avgWaiting=(1.0*totalWaiting/numberOfProcess);
        double avgturnaround=1.0*totalTurnAround/numberOfProcess;
        System.out.print("SJF ");
        System.out.printf("%.2f ,",avgWaiting);
        System.out.printf("%.2f ,",(avgturnaround));

        int totPageFault=0;
        for(int i=0;i<numberOfProcess;i++)
            totPageFault+=pList[i].pageFault;


        System.out.print(totPageFault+" ,");

        for(int i=0;i<numberOfProcess;i++)
            System.out.print(pList[i].pageFault+" ,");

        double pf= (fcfsturn-avgturnaround)/avgturnaround*100;
        System.out.printf("%.2f \n",pf);

        return avgturnaround;


    }

}
