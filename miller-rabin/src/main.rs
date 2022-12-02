use rand::Rng;
use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();
    let argument_count = args.len() - 1; // first is the executable name

    match argument_count {
        0 => {
            println!("miller-rabin <prime> <rounds>");
            println!("use <rounds> rounds of Miller-Rabin to test whether <prime> is prime");
            return;
        }
        2 => {}
        _ => {
            println!("Unexpected number of arguments: {argument_count}");
            return;
        }
    }

    // TODO: move to bigint
    let prime: u128 = args[1].parse().expect("Need a number!");
    let rounds: u32 = args[2].parse().expect("Need a number!");

    println!("Checking to see whether {prime} is prime using {rounds} round(s) of Miller-Rabin");

    //  Handle even numbers and numbers less than 5 by hand
    if prime < 5 || (prime % 2) == 0 {
        match prime {
            2 | 3 => println!("{prime} is DEFINITELY PRIME"),
            4 => println!("{prime} is DEFINITELY COMPOSITE"),
            _ => println!("{prime} is DEFINITELY NOT PRIME")
        }
        return;
    } else {
        // p is an odd prime >= 5
        // so p - 1 is even and has at least one factor of two
        // pull out as many factors of 2 from (p - 1) as possible
        // write it as p = 2^s d + 1 where d is an odd number

        let mut d = prime - 1;
        let mut s: u128 = 0;

        while (d % 2) == 0 {
            d /= 2;
            s += 1;
        }

        // Now we go looking for witnesses
        let mut any_composite: bool = false;
        let mut rng = rand::thread_rng();

        // TODO: is there a way to suppress the unneeded _r variable?
        for _r in 1 ..= rounds {
            // Choose a random integer a from 2 to prime - 2 inclusive
            let a = rng.gen_range(2 .. prime - 1);

            // Calculate a^d, a^(2d), a^(4d), a^(8d), ... a^(2^(s - 1) d)
            // That is, calculate a^(2^r d) for r = 0, 1, ..., s - 1
            // a^((2^s)d) = a^(p - 1) = 1 so we don't bother with that
            let mut a_2_r_d: u128 = 1;

            // a_2_r_d = a^d mod prime
            // TODO: use modpow crate instead of reimplementing
            let mut a_temp = a;
            let mut d_temp = d;
            while d_temp != 0 {
                if (d_temp % 2) == 1 {
                    a_2_r_d *= a_temp;
                    a_2_r_d %= prime;
                }

                d_temp /= 2;
                a_temp *= a_temp;
                a_temp %= prime;
            }

            let mut any_1_or_negative_1: bool = false;
            for r in 0 .. s {
                a_2_r_d = (a_2_r_d * a_2_r_d) % prime;

                // If this is 1 or p - 1 then we won't get any further information
                // from this witness
                // p COULD BE prime but we should talk to other witnesses
                if (a_2_r_d == 1) || (a_2_r_d == (prime - 1)) {
                    println!("{a}^(2^{r} {d}) = {a_2_r_d} witnesses that {prime} COULD BE PRIME");
                    any_1_or_negative_1 = true;
                    break;
                }
            }

            if !any_1_or_negative_1 {
                println!("{a}^(2^r {d}) with r = 0 to ({s} - 1) has no 1 or -1 so {prime} is DEFINITELY COMPOSITE");
                any_composite = true;
                break;
            }
        }

        if !any_composite {
            println!("All witnesses state that {prime} COULD BE PRIME");
        }
    }
}
