/*
 * Function(s) for dealing with potions
 *
 * potions.c	1.4 (AI Design)		2/12/84
 */

#include "rogue.h"
#include "curses.h"


//@ turn_see() wrapper to use as a fuse
static
void
turn_see_off(void)
{
	turn_see(TRUE);
}

/*
 * quaff:
 *	Quaff a potion from the pack
 */
void
quaff(void)
{
	register THING *obj, *th;
	register bool discardit = FALSE;

	if ((obj = get_item("quaff", POTION)) == NULL)
		return;
	/*
	 * Make certain that it is somethings that we want to drink
	 */
	if (obj->o_type != POTION)
	{
		msg("yuk! Why would you want to drink that?");
		return;
	}
	if (obj == cur_weapon)
		cur_weapon = NULL;

	/*
	 * Calculate the effect it has on the poor guy.
	 */
	switch (obj->o_which)
	{
	when P_CONFUSE:
		p_know[P_CONFUSE] = TRUE;
		if (!on(player, ISHUH))
			{
			if (on(player, ISHUH))
				lengthen(unconfuse, rnd(8)+HUHDURATION);
			else
				fuse(unconfuse, rnd(8)+HUHDURATION);
			player.t_flags |= ISHUH;
			msg("wait, what's going on? Huh? What? Who?");
		}
	when P_POISON:
		{
		char *sick = "you feel %s sick.";

		p_know[P_POISON] = TRUE;
		if (!ISWEARING(R_SUSTSTR))
		{
			chg_str(-(rnd(3)+1));
			msg(sick, "very");
		}
		else
			msg(sick, "momentarily");
		}
	when P_HEALING:
		p_know[P_HEALING] = TRUE;
		if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
			pstats.s_hpt = ++max_hp;
		sight();
		msg("you begin to feel better");
	when P_STRENGTH:
		p_know[P_STRENGTH] = TRUE;
		chg_str(1);
		msg("you feel stronger. What bulging muscles!");
	when P_MFIND:
#ifndef DEMO
		fuse(turn_see_off, HUHDURATION);
		if (mlist == NULL)
			msg("you have a strange feeling%s.",
				noterse(" for a moment"));
		else
		{
			if (turn_see(FALSE))
			{
				p_know[P_MFIND] = TRUE;
			}
			msg("");
		}
#else
		msg("you can't move");
		msg(" and are forced to watch this advertisement");
		msg("rogue: The ULTIMATE Adventure Game");
		msg("the most popular game on UNIX ever!");
		msg("now runs on YOUR IBM PC");
		msg("UNIX is a trademark of Bell Labs");
		p_know[P_MFIND] = TRUE;
#endif
	  when P_TFIND:
		/*
		 * Potion of magic detection.  Find everything interesting on
		 * the level and show him where they are.  Also give hints as
		 * to whether he would want to use the object.
		 */
		if (lvl_obj != NULL)
		{
			register THING *tp;
			register bool show;

			show = FALSE;
			for (tp = lvl_obj; tp != NULL; tp = next(tp))
			{
				if (is_magic(tp))
				{
					show = TRUE;
					//mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, goodch(tp));
					move_and_draw_tile(tp->o_pos.y, tp->o_pos.x, goodch(tp));
					p_know[P_TFIND] = TRUE;
				}
			}
			for (th = mlist; th != NULL; th = next(th))
			{
				for (tp = th->t_pack; tp != NULL; tp = next(tp))
				{
					if (is_magic(tp))
					{
						show = TRUE;
						//mvwaddch(hw, th->t_pos.y, th->t_pos.x, MAGIC);
						move_and_draw_tile(th->t_pos.y, th->t_pos.x, MAGIC);
						p_know[P_TFIND] = TRUE;
					}
				}
			}
			if (show)
			{
				msg("You sense the presence of magic.");
				break;
			}
		}
		msg("you have a strange feeling for a moment%s.",
				noterse(", then it passes"));
	when P_PARALYZE:
		p_know[P_PARALYZE] = TRUE;
		no_command = HOLDTIME;
		player.t_flags &= ~ISRUN;
		msg("you can't move");
	when P_SEEINVIS:
		if (!on(player, CANSEE)) {
			fuse(unsee, SEEDURATION);
			look(FALSE);
			invis_on();
		}
		sight();
		msg("this potion tastes like %s juice", fruit);
	when P_RAISE:
		p_know[P_RAISE] = TRUE;
		msg("you suddenly feel much more skillful");
		raise_level();
	when P_XHEAL:
		p_know[P_XHEAL] = TRUE;
		if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		{
			if (pstats.s_hpt > max_hp + pstats.s_lvl + 1)
				++max_hp;
			pstats.s_hpt = ++max_hp;
		}
		sight();
		msg("you begin to feel much better");
	when P_HASTE:
		p_know[P_HASTE] = TRUE;
		if (add_haste(TRUE))
			msg("you feel yourself moving much faster");
	when P_RESTORE:
		if (ISRING(LEFT, R_ADDSTR))
			add_str(&pstats.s_str, -cur_ring[LEFT]->o_ac);
		if (ISRING(RIGHT, R_ADDSTR))
			add_str(&pstats.s_str, -cur_ring[RIGHT]->o_ac);
		if (pstats.s_str < max_stats.s_str)
			pstats.s_str = max_stats.s_str;
		if (ISRING(LEFT, R_ADDSTR))
			add_str(&pstats.s_str, cur_ring[LEFT]->o_ac);
		if (ISRING(RIGHT, R_ADDSTR))
			add_str(&pstats.s_str, cur_ring[RIGHT]->o_ac);
		msg("%syou feel warm all over",
			noterse("hey, this tastes great.  It makes "));
	when P_BLIND:
		p_know[P_BLIND] = TRUE;
		if (!on(player, ISBLIND))
		{
			player.t_flags |= ISBLIND;
			fuse(sight, SEEDURATION);
			look(FALSE);
		}
		msg("a cloak of darkness falls around you");
	when P_NOP:
		msg("this potion tastes extremely dull");
	otherwise:
		msg("what an odd tasting potion!");
		return;
	}
	status();
	/*
	 * Throw the item away
	 */
	inpack--;
	if (obj->o_count > 1)
		obj->o_count--;
	else
	{
		detach(pack, obj);
		discardit = TRUE;
	}

	call_it(p_know[obj->o_which], &p_guess[obj->o_which]);

	if (discardit)
		discard(obj);
}

/*
 * invis_on:
 *	Turn on the ability to see invisible
 */
void
invis_on(void)
{
	register THING *th;

	player.t_flags |= CANSEE;
	for (th = mlist; th != NULL; th = next(th))
	if (on(*th, ISINVIS) && see_monst(th))
	{
		//mvaddch(th->t_pos.y, th->t_pos.x,th->t_disguise);
		move_and_draw_tile(th->t_pos.y, th->t_pos.x,th->t_disguise);
	}
}

/*
 * turn_see:
 *	Put on or off seeing monsters on this level
 */
bool
turn_see(bool turn_off)
{
	register THING *mp;
	register bool can_see, add_new;
	byte was_there = inch();

	add_new = FALSE;
	for (mp = mlist; mp != NULL; mp = next(mp)) {
		move(mp->t_pos.y, mp->t_pos.x);
		can_see = (see_monst(mp) || (was_there = inch()) == mp->t_type);
		if (turn_off) {
			if (!see_monst(mp) && mp->t_oldch != '@')
				//addch(mp->t_oldch);dra
				draw_tile(mp->t_oldch);
		} else {
			if (!can_see) {
				standout();
				mp->t_oldch = was_there;
			}
			//addch(mp->t_type);
			draw_tile(mp->t_type);
			if (!can_see) {
				standend();
				add_new = TRUE;
			}
		}
	}
	player.t_flags |= SEEMONST;
	if (turn_off)
		player.t_flags &= ~SEEMONST;
	return add_new;
}

/*
 * th_effect:
 *	Compute the effect of this potion hitting a monster.
 */
void
th_effect(THING *obj, THING *tp)
{
	switch (obj->o_which)
	{
	when P_CONFUSE:
	case P_BLIND:
		tp->t_flags |= ISHUH;
		msg("the %s appears confused", monsters[tp->t_type-'A'].m_name);
	when P_PARALYZE:
		tp->t_flags &= ~ISRUN;
		tp->t_flags |= ISHELD;
	when P_HEALING:
	case P_XHEAL:
		if ((tp->t_stats.s_hpt += rnd(8)) > tp->t_stats.s_maxhp)
		tp->t_stats.s_hpt = ++tp->t_stats.s_maxhp;
	when P_RAISE:
		tp->t_stats.s_hpt += 8;
		tp->t_stats.s_maxhp += 8;
		tp->t_stats.s_lvl++;
	when P_HASTE:
		tp->t_flags |= ISHASTE;
		break;
	}
	msg("the flask shatters.");
}
