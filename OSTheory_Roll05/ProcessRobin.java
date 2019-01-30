import javafx.util.Pair;

import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedList;
import java.util.Queue;

class ProcessRobin {

    int id;
    int numberOfPages;
    int arrivalTime;
    int actualArrival;
    Deque<Pair<Integer,Integer>> memRef = new LinkedList<>();
    ArrayList<Integer> pageTable = new ArrayList<Integer>();
    int tablesize;
    int pageFault;
    int waitingTime;
    int executionTime;
    int totalTime;
    int quanta;

    int pageFaultOccured;


    boolean isPageFault() {

        Pair currentmem = memRef.peek();

        if (!pageTable.contains((Integer)currentmem.getKey())) {
            return true;
        } else {

            return false;
        }

    }


    void doPageFault() {

        Pair currentmem = memRef.peek();

        tablesize= (numberOfPages-1)/3+1;

        if (pageTable.size() >= tablesize) {

            pageTable.remove(0);
        }

        pageTable.add((Integer)currentmem.getKey());
        pageFault++;

    }


    void noPageFault(int allocatedTime) {

        Pair currentmem = memRef.peek();
        memRef.remove();
        if((Integer)currentmem.getValue()-allocatedTime>0) {
            Pair updatedmem = new Pair(currentmem.getKey(), (Integer) currentmem.getValue() - allocatedTime);
            memRef.addFirst(updatedmem);
        }



        for (int j = 0; j < pageTable.size(); j++) {
            if (pageTable.get(j) == (Integer)currentmem.getKey())
                pageTable.remove(j);
        }

        pageTable.add((Integer)currentmem.getKey());


    }

}
