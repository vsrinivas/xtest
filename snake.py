import curses
import random
import time

def main(stdscr):
    # Initialize curses
    curses.curs_set(0)  # Hide cursor
    stdscr.nodelay(1)   # Non-blocking input
    stdscr.timeout(100) # Refresh every 100 ms

    # Get screen dimensions
    sh, sw = stdscr.getmaxyx()
    # Create initial snake coordinates
    snake = [
        [sh//2, sw//4],
        [sh//2, sw//4 - 1],
        [sh//2, sw//4 - 2]
    ]
    # Initial direction: right
    direction = curses.KEY_RIGHT

    # Place first food
    food = [random.randint(1, sh-2), random.randint(1, sw-2)]
    while food in snake:
        food = [random.randint(1, sh-2), random.randint(1, sw-2)]
    stdscr.addch(food[0], food[1], curses.ACS_PI)

    score = 0

    while True:
        # Get user input
        key = stdscr.getch()
        if key in [curses.KEY_RIGHT, curses.KEY_LEFT, curses.KEY_UP, curses.KEY_DOWN]:
            # Prevent snake from reversing
            if (direction == curses.KEY_RIGHT and key != curses.KEY_LEFT) or \
               (direction == curses.KEY_LEFT and key != curses.KEY_RIGHT) or \
               (direction == curses.KEY_UP and key != curses.KEY_DOWN) or \
               (direction == curses.KEY_DOWN and key != curses.KEY_UP):
                direction = key

        # Calculate new head position
        head = snake[0][:]
        if direction == curses.KEY_RIGHT:
            head[1] += 1
        elif direction == curses.KEY_LEFT:
            head[1] -= 1
        elif direction == curses.KEY_UP:
            head[0] -= 1
        elif direction == curses.KEY_DOWN:
            head[0] += 1

        # Check for collision with walls
        if head[0] in [0, sh] or head[1] in [0, sw]:
            msg = f"Game Over! Score: {score}"
            stdscr.addstr(sh//2, sw//2 - len(msg)//2, msg)
            stdscr.nodelay(0)
            stdscr.getch()
            break

        # Check for collision with self
        if head in snake:
            msg = f"Game Over! Score: {score}"
            stdscr.addstr(sh//2, sw//2 - len(msg)//2, msg)
            stdscr.nodelay(0)
            stdscr.getch()
            break

        # Insert new head
        snake.insert(0, head)

        # Check if food eaten
        if head == food:
            score += 1
            # Place new food
            food = [random.randint(1, sh-2), random.randint(1, sw-2)]
            while food in snake:
                food = [random.randint(1, sh-2), random.randint(1, sw-2)]
            stdscr.addch(food[0], food[1], curses.ACS_PI)
        else:
            # Remove tail
            tail = snake.pop()
            stdscr.addch(tail[0], tail[1], ' ')

        # Draw snake
        stdscr.addch(snake[0][0], snake[0][1], curses.ACS_CKBOARD)

        # Draw score
        score_text = f"Score: {score}"
        stdscr.addstr(0, 2, score_text)

        stdscr.refresh()

if __name__ == "__main__":
    curses.wrapper(main)
