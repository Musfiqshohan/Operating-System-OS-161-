import java.util.Comparator;

class ShortestJob implements Comparator<Process> {
    @Override
    public int compare(Process o1, Process o2) {

            return (o1.totalTime-o2.totalTime);
    }


}
  