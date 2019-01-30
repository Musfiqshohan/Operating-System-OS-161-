import javafx.util.Pair;

import java.io.FileReader;
import java.util.*;




public class RoundRobin {


    public double run(double fcfsturn) throws Exception{



        FileReader fin = new FileReader("input.txt");
        Scanner sc = new Scanner(fin);


        String line = sc.nextLine();
        line = line.replace(" ", "");

        StringTokenizer st = new StringTokenizer(line, ",");

        int numberOfProcess = Integer.parseInt(st.nextToken());
        int timeQuanta = Integer.parseInt(st.nextToken());


        ProcessRobin pList[] = new ProcessRobin[numberOfProcess];

        for (int i = 0; i < numberOfProcess; i++) {

            line = sc.nextLine();
            line = line.replace(" ", "");
            st = new StringTokenizer(line, ",");


            pList[i] = new ProcessRobin();
            pList[i].id = Integer.parseInt(st.nextToken());
            pList[i].numberOfPages = Integer.parseInt(st.nextToken());
            pList[i].arrivalTime = Integer.parseInt(st.nextToken());
            pList[i].actualArrival = pList[i].arrivalTime;


            while (st.hasMoreTokens()) {

                Pair mem=new Pair((Integer.parseInt(st.nextToken())) / 512,30);
                pList[i].memRef.add(mem);

            }


            int pageFault = pList[i].pageFault;

            pList[i].waitingTime = pageFault * 60;
            pList[i].executionTime = pList[i].memRef.size() * 30;
            pList[i].totalTime = pList[i].waitingTime + pList[i].executionTime;
            pList[i].pageFaultOccured=-1;
            pList[i].quanta=timeQuanta;

        }


        PriorityQueue<ProcessRobin> pq = new PriorityQueue<ProcessRobin>(new RoundRobinComp());



        for (int i = 0; i < numberOfProcess; i++) {

            pq.add(pList[i]);
        }


        int blockingTime=pq.peek().arrivalTime;
        int cpuTime=pq.peek().arrivalTime;

        int totalWaiting=0,totalTurnAround=0;

        int cnt=0;

        while (!pq.isEmpty()){



            ProcessRobin curProcess= pq.peek();
            pq.remove();



            cpuTime= Math.max(cpuTime, curProcess.arrivalTime);


            if(curProcess.isPageFault()==true){

                blockingTime= Math.max(blockingTime, cpuTime);
                blockingTime+=60;

                curProcess.arrivalTime=blockingTime;
                curProcess.pageFaultOccured=1;
                curProcess.doPageFault();
                pq.add(curProcess);

            }
            else{

                int allocatedTime=timeQuanta;  //for current process
                while(curProcess.memRef.size()!=0  && curProcess.isPageFault()==false && curProcess.memRef.peekFirst().getValue()<allocatedTime){
                    int memTime=curProcess.memRef.peekFirst().getValue();
                    cpuTime+=memTime;
                    curProcess.noPageFault(allocatedTime);
                    allocatedTime-=memTime;


                }




                if(curProcess.memRef.isEmpty()==false && curProcess.isPageFault()==true){

                    blockingTime= Math.max(blockingTime, cpuTime);
                    blockingTime+=60;

                    curProcess.arrivalTime=blockingTime;
                    curProcess.pageFaultOccured=1;
                    curProcess.doPageFault();
                    pq.add(curProcess);


                }

                else if(curProcess.memRef.isEmpty()==false && curProcess.memRef.peekFirst().getValue()>=allocatedTime){
                    cpuTime+=allocatedTime;
                    curProcess.arrivalTime=cpuTime;
                    curProcess.pageFaultOccured=-1;
                    curProcess.noPageFault(allocatedTime);


                    if(!curProcess.memRef.isEmpty())
                         pq.add(curProcess);

                }


                if(curProcess.memRef.isEmpty()){
                    totalWaiting+= (cpuTime - curProcess.actualArrival- curProcess.executionTime);
                    totalTurnAround+= (cpuTime - curProcess.actualArrival);
                }




            }

        }




        double avgWaiting=(1.0*totalWaiting/numberOfProcess);
        double avgturnaround=1.0*totalTurnAround/numberOfProcess;
        System.out.print("RRS ");
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
