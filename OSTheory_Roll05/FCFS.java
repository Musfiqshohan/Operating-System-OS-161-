import java.io.FileReader;
import java.util.*;




public class FCFS {


    public double run() throws Exception{


        FileReader fin = new FileReader("input.txt");
        Scanner sc = new Scanner(fin);


        String line = sc.nextLine();
        line = line.replace(" ", "");

        StringTokenizer st = new StringTokenizer(line, ",");

        int numberOfProcess = Integer.parseInt(st.nextToken());
        int timeQuanta = Integer.parseInt(st.nextToken());


        ProcessFCFS pList[] = new ProcessFCFS[numberOfProcess];

        for (int i = 0; i < numberOfProcess; i++) {

            line = sc.nextLine();
            line = line.replace(" ", "");
            st = new StringTokenizer(line, ",");


            pList[i] = new ProcessFCFS();
            pList[i].id = Integer.parseInt(st.nextToken());
            pList[i].numberOfPages = Integer.parseInt(st.nextToken());
            pList[i].arrivalTime = Integer.parseInt(st.nextToken());
            pList[i].actualArrival = pList[i].arrivalTime;


            while (st.hasMoreTokens()) {


                pList[i].memRef.add((Integer.parseInt(st.nextToken())) / 512);

            }



            int pageFault = pList[i].pageFault;

            pList[i].waitingTime = pageFault * 60;
            pList[i].executionTime = pList[i].memRef.size() * 30;
            pList[i].totalTime = pList[i].waitingTime + pList[i].executionTime;

        }


        PriorityQueue<ProcessFCFS> pq = new PriorityQueue<ProcessFCFS>(new FirstCome());



        for (int i = 0; i < numberOfProcess; i++) {
            pq.add(pList[i]);
        }


        int blockingTime=pq.peek().arrivalTime;
        int cpuTime=pq.peek().arrivalTime;

        int totalWaiting=0,totalTurnAround=0;

        while (!pq.isEmpty()){


            ProcessFCFS curProcess= pq.peek();
            pq.remove();



            cpuTime= Math.max(cpuTime, curProcess.arrivalTime);



            if(curProcess.isPageFault()==true){

                blockingTime= Math.max(blockingTime, cpuTime);
                blockingTime+=60;

                curProcess.arrivalTime=blockingTime;

                pq.add(curProcess);
                curProcess.doPageFault();

            }
            else{

                cpuTime+=30;

                curProcess.noPageFault();


                while (curProcess.memRef.size()!=0  && curProcess.isPageFault()==false){
                    curProcess.noPageFault();

                    cpuTime+=30;
                }


                if(curProcess.memRef.isEmpty()){
                    totalWaiting+= (cpuTime - curProcess.actualArrival- curProcess.executionTime);

                    totalTurnAround+= (cpuTime - curProcess.actualArrival);
                }


                if(curProcess.memRef.size()!=0  && curProcess.isPageFault()==true){

                    blockingTime= Math.max(blockingTime, cpuTime);
                    blockingTime+=60;

                    curProcess.arrivalTime=blockingTime;


                    pq.add(curProcess);
                    curProcess.doPageFault();

                }







            }

        }



        double avgWaiting=(1.0*totalWaiting/numberOfProcess);
        double avgturnaround=1.0*totalTurnAround/numberOfProcess;
        System.out.print("FCFS ");
        System.out.printf("%.2f ,",avgWaiting);
        System.out.printf("%.2f ,",(avgturnaround));

        int totPageFault=0;
        for(int i=0;i<numberOfProcess;i++)
            totPageFault+=pList[i].pageFault;


        System.out.print(totPageFault+" ,");

        for(int i=0;i<numberOfProcess;i++)
            System.out.print(pList[i].pageFault+" ,");

        double pf= 0;
        System.out.printf("%.2f \n",pf);

        return avgturnaround;

    }
}
