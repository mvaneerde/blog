
using System.Configuration.Assemblies;
using System.Transactions;

namespace LinkedIn
{
    public class Node
    {
        public Node? next;
        public Char value;

        public Node(Char c)
        {
            next = null;
            value = c;
        }

        public Node(string s)
        {
            value = s[0];

            Node? latest = this;
            for (int i = 1; i < s.Length; i++)
            {
                latest.next = new Node(s[i]);
                latest = latest.next;
            }
        }

        public string AsString()
        {
            string s = "";
            for (var current = this; current != null; current = current.next)
            {
                s += current.value;
            }

            return s;
        }
    }

    class Program
    {
        static Node? ModifyList(Node list, Node left, Node right)
        {
            // * -> * -> L -> * -> * -> R -> * -> *
            // leave everything where it is and tweak the arrows
            //
            //       ------------------>
            // * -> *    L <- * <- * <- R    * -> *
            //            ------------------>
            //
            // algorithm:
            // 1. Up to but not including L
            //     if L is the first node in the list:
            //        the new beginning is R, return this instead of list
            //     otherwise
            //        find the node BEFORE L
            //        set before_L.next = R
            if (list == left)
            {
                Console.WriteLine("Left {0} is the beginning of the list, changing it to right {1}", left.value, right.value);
                list = right;
            }
            else
            {
                for (var before_l = list; before_l != null; before_l = before_l.next)
                {
                    if (before_l.next == left)
                    {
                        Console.WriteLine("Node before left {0} is {1}, changing next to right {2}", before_l.value, left.value, right.value);
                        before_l.next = right;
                        break;
                    }
                }
            }

            // 2. L
            //     remember l.next (call it "following")
            //     set L.next = R.next
            //     remember l (call it "previous")
            var following = left.next;
            left.next = right.next;
            var previous = left;

            // 3. from one after L to R inclusive
            //     start with current = following and loop until current = right
            //     remember current.next as following
            //     set current.next to previous
            //     set current to following
            //     after we break out of the loop, set right.next to previous
            for (var current = following; current != null && current != right; current = following)
            {
                following = current.next;
                current.next = previous;
                previous = current;
            }
            
            right.next = previous;

            // 4. from one after R to the end - there is nothing to do
            return list;
        }

        static void Main(string[] args)
        {
            switch (args.Length)
            {
                case 0:
                    {
                        Console.WriteLine("LinkedList.exe ABCDE B D");
                        Console.WriteLine("builds a linked list from ABCDE with five elements");
                        Console.WriteLine("then reverses the portion from B to D to get ADCBE");
                        Console.WriteLine("finally prints the linked list as a string");
                        break;

                    }

                case 3:
                    {
                        // we expect three arguments
                        // 1. a string like ABCDE
                        // 2. a letter in the string like B
                        // 3. a later letter in the string like D
                        var listString = args[0];
                        var leftLetter = args[1];
                        var rightLetter = args[2];

                        if (leftLetter.Length != 1)
                        {
                            Console.WriteLine("left letter should be a single letter, not {0}", leftLetter);
                            return;
                        }

                        if (rightLetter.Length != 1)
                        {
                            Console.WriteLine("right letter should be a single letter, not {0}", rightLetter);
                            return;
                        }

                        if (leftLetter == rightLetter)
                        {
                            Console.WriteLine("Left letter {0} and right letter {1} need to be different", leftLetter, rightLetter);
                            return;
                        }

                        // build a linked list out of the given string
                        var list = new Node(listString);

                        // write the output back just as we read it
                        Console.WriteLine("Linked list before modification: {0}", list.AsString());

                        // find the nodes and verify they are in the right order
                        Node? left = null;
                        Node? right = null;
                        for (var current = list; current != null; current = current.next)
                        {
                            if (current.value == leftLetter[0])
                            {
                                if (left == null)
                                {
                                    left = current;
                                }
                                else
                                {
                                    Console.WriteLine("Left letter {0} is not unique", leftLetter);
                                    return;
                                }
                            }

                            if (current.value == rightLetter[0])
                            {
                                if (left == null)
                                {
                                    Console.WriteLine("Right letter {0} came before left {1}", rightLetter, leftLetter);
                                    return;
                                }
                                else if (right == null)
                                {
                                    right = current;
                                }
                                else
                                {
                                    Console.WriteLine("Right letter {0} is not unique", rightLetter);
                                    return;
                                }
                            }
                        }

                        if (left == null)
                        {
                            Console.WriteLine("Could not find left letter {0}", leftLetter);
                            return;
                        }

                        if (right == null)
                        {
                            Console.WriteLine("Could not find right letter {0}", rightLetter);
                            return;
                        }

                        // modify the linked list to reverse the segment connecting the two letters
                        list = ModifyList(list, left, right);

                        // finally print the modified linked list
                        Console.Write("Linked list after modification: {0}", list.AsString());
                        break;
                    }

                default:
                    {
                        Console.WriteLine("Unexpected number of arguments: {0}", args.Length);
                        break;
                    }
            }
        }
    }
}