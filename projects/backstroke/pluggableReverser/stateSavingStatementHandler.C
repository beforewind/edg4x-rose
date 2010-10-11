#include "stateSavingStatementHandler.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <utilities/Utilities.h>
#include <utilities/CPPDefinesAndNamespaces.h>

using namespace boost::lambda;
using namespace SageBuilder;
using namespace SageInterface;


#if 0

/** Get the left most variable. For example, a.b returns a, a->b returns a. */
SgVarRefExp* getMostLeftVariable(SgExpression* exp)
{
	if (SgVarRefExp* var_ref = isSgVarRefExp(exp))
		return var_ref;
	if (SgDotExp* dot_exp = isSgDotExp(exp))
		return getMostLeftVariable(dot_exp->get_lhs_operand());
	if (SgArrowExp* arrow_exp = isSgArrowExp(exp))
		return getMostLeftVariable(arrow_exp->get_lhs_operand());
	return NULL;
}

vector<SgExpression*> getAllModifiedVariables(SgStatement* stmt)
{
	vector<SgExpression*> modified_vars;

	vector<SgExpression*> exps = backstroke_util::querySubTree<SgExpression>(stmt);
	foreach (SgExpression* exp, exps)
	{
		SgExpression* var = NULL;

		if (SageInterface::isAssignmentStatement(exp))
		{
			var = isSgBinaryOp(exp)->get_lhs_operand();
		}
		else if (isSgPlusPlusOp(exp) || isSgMinusMinusOp(exp))
		{
			var = isSgUnaryOp(exp)->get_operand();
		}
		else if (isSgFunctionCallExp(exp))
		{
			// This part should be refined.
			ROSE_ASSERT(false);
		}

		if (var)
		{
			if (SgVarRefExp* var_ref = getMostLeftVariable(var))
			{
				// Get the declaration of this variable to see if it's declared inside of the given statement.
				// In this case, we don't have to store this variable.
				SgDeclarationStatement* decl = var_ref->get_symbol()->get_declaration()->get_declaration();
				if (!isAncestor(stmt, decl))
				{
					// We store each variable once.
					if (boost::find_if(modified_vars, bind(backstroke_util::areSameVariable, _1, var)) == modified_vars.end())
						modified_vars.push_back(var);
				}
			}
		}
	}

	return modified_vars;
}

#endif



vector<VariableRenaming::VarName> StateSavingStatementHandler::getAllModifiedVariables(SgStatement* stmt)
{
	vector<VariableRenaming::VarName> modified_vars;
	foreach (const VariableRenaming::NumNodeRenameTable::value_type& num_node,
			getVariableRenaming()->getOriginalDefsForSubtree(stmt))
			modified_vars.push_back(num_node.first);
	return modified_vars;
}

bool StateSavingStatementHandler::checkStatement(SgStatement* stmt) const
{
	if (isSgWhileStmt(stmt) ||
		isSgIfStmt(stmt) ||
		isSgDoWhileStmt(stmt) ||
		isSgForStatement(stmt) ||
		isSgSwitchStatement(stmt))
		return true;

	if (isSgBasicBlock(stmt))
	{
		SgNode* parent_stmt = stmt->get_parent();
		if (isSgWhileStmt(parent_stmt) ||
			isSgDoWhileStmt(parent_stmt) ||
			isSgForStatement(parent_stmt) ||
			isSgSwitchStatement(parent_stmt))
			return false;
		else
			return true;
	}
	return false;
}

StatementReversal StateSavingStatementHandler::generateReverseAST(SgStatement* stmt, const EvaluationResult& eval_result)
{
	SgBasicBlock* fwd_stmt = buildBasicBlock();
    SgBasicBlock* rvs_stmt = buildBasicBlock();

	// If the following child result is empty, we don't have to reverse the target statement.
	vector<EvaluationResult> child_result = eval_result.getChildResults();
	if (!child_result.empty())
	{
		StatementReversal child_reversal = child_result[0].generateReverseStatement();
		prependStatement(child_reversal.fwd_stmt, fwd_stmt);
		appendStatement(child_reversal.rvs_stmt, rvs_stmt);
	}
	else
	{
		prependStatement(copyStatement(stmt), fwd_stmt);
	}

	vector<VariableRenaming::VarName> modified_vars = eval_result.getAttribute<vector<VariableRenaming::VarName> >();
	foreach (const VariableRenaming::VarName& var_name, modified_vars)
	{
		SgExpression* var = VariableRenaming::buildVariableReference(var_name);
		SgExpression* fwd_exp = pushVal(var);
		SgExpression* rvs_exp = buildBinaryExpression<SgAssignOp>(
			copyExpression(var), popVal(var->get_type()));
		
		prependStatement(buildExprStatement(fwd_exp), fwd_stmt);
		appendStatement(buildExprStatement(rvs_exp), rvs_stmt);
	}

	return StatementReversal(fwd_stmt, rvs_stmt);
}

std::vector<EvaluationResult> StateSavingStatementHandler::evaluate(SgStatement* stmt, const VariableVersionTable& var_table)
{
	vector<EvaluationResult> results;

	// Currently, we just perform this state saving handler on if/while/for/do-while/switch statements and pure
	// basic block which is not the body of if/while/for/do-while/switch statements.
	if (!checkStatement(stmt))
		return results;

	// In case of infinite calling to this function.
	if (evaluating_stmts_.count(stmt) > 0)
		return results;

	//vector<SgExpression*> modified_vars = getAllModifiedVariables(stmt);

#if 0
	cout << "\n\n";
	var_table.print();
	cout << "\n\n";
#endif

	vector<VariableRenaming::VarName> modified_vars = getAllModifiedVariables(stmt);
	VariableVersionTable new_table = var_table;
	new_table.reverseVersionAtStatementStart(stmt);

#if 0
	cout << "\n\n";
	new_table.print();
	cout << "\n\n";
#endif

	// Reverse the target statement using other handlers.
	evaluating_stmts_.insert(stmt);
	vector<EvaluationResult> eval_results = evaluateStatement(stmt, var_table);
	evaluating_stmts_.erase(stmt);

	// We combine both state saving and reversed target statement together.
	// In a following analysis on generated code, those extra store and restores will be removed.
	foreach (const EvaluationResult& eval_result, eval_results)
	{
		EvaluationResult result(this, stmt, new_table);
		result.addChildEvaluationResult(eval_result);
		// Add the attribute to the result.
		result.setAttribute(modified_vars);
		results.push_back(result);
	}

	// Here we just use state saving.
	
	EvaluationResult result(this, stmt, new_table);
	// Add the attribute to the result.
	result.setAttribute(modified_vars);
	results.push_back(result);

	return results;
}