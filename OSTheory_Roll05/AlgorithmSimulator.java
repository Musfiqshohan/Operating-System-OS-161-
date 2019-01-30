public class AlgorithmSimulator {


    public static void main(String[] args) throws  Exception {



        FCFS fcfs= new FCFS();
        SJF sjf= new SJF();
        RoundRobin roundRobin=new RoundRobin();
        double avgturnaround=fcfs.run();

        sjf.run(avgturnaround);
        roundRobin.run(avgturnaround);






    }
}
